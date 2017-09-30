import os

import tensorflow as tf
import pandas as pd
from functools import partial
import time
from util import *

from hist_rnn import rnn_with_hist_loop_fn
from config import Config
from env import Env

from model import LCACell, normalize_weights, exp_poisson
from tensorflow.contrib.rnn import MultiRNNCell

import scikits.statsmodels.tsa.api as smt
from scikits.statsmodels.tsa.arima_process import arma_generate_sample

from ts_pp import white_ts, generate_ts

from util import *



lrate = 0.01
epochs = 1

tf.set_random_seed(1)
np.random.seed(1)

input_size = 2
seq_size = 500
batch_size = 3
layer_size = 25
filter_len = 25

dt = 1.0

c = Config()
c.lam = 0.01
c.weight_init_factor = 0.5
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0/seq_size
c.simple_hebb = True
c.tau_m = 50.0
c.adapt = 5.0
c.act_factor = 1.0
c.adaptive = False


def transform_output(x_hat_flat):
    input_ta = tf.TensorArray(dtype=tf.float32, size=seq_size, tensor_array_name="input") #, clear_after_read=False)
    input_ta = input_ta.unstack(x_hat_flat)

    i = tf.constant(0, dtype=tf.int32, name="i")
    output_ta = tf.TensorArray(dtype=tf.float32, size=seq_size, tensor_array_name="output")

    def _time_step(i, input_ta_t, output_ta_t):
        left_i = tf.maximum(0, i-filter_len)

        x_t_flat = input_ta_t.read(i)
        x_t = tf.reshape(x_t_flat, (batch_size, filter_len, input_size))
        
        x_t_sliced = tf.slice(x_t, (0,0,0), (batch_size, i-left_i, input_size))
        x_t_sliced = tf.pad(x_t_sliced, [[0,0], [filter_len - (i-left_i), 0], [0,0]])
        
        x_t_sliced = tf.transpose(x_t_sliced, (1, 0, 2))/(c.tau*2)
        

        output_ta_t = output_ta_t.write(i, x_t_sliced)
        return i+1, input_ta_t, output_ta_t

    _, _, output_final_ta = tf.while_loop(
        cond=lambda i, *_: i < seq_size,
        body=_time_step,
        loop_vars=(i, input_ta, output_ta),
    )

    return output_final_ta.stack()



input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")
sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)

net = MultiRNNCell([
    LCACell(input_size, layer_size, filter_len, c, tf.nn.relu),
])

state = tuple(
    tuple((
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="u"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a_m"),
        tf.placeholder(tf.float32, [batch_size, cell.filter_len*cell.input_size, cell.layer_size], name="dF"),
    )) for cell in net._cells
)

get_zero_state = lambda: tuple(
    np.zeros((batch_size,) + tuple(t.get_shape().as_list()[1:]))
    for tup in state for t in tup
)


(u_ta, a_ta, a_m_ta, x_hat_flat_ta), finstate, _ = tf.nn.raw_rnn(
    net, 
    rnn_with_hist_loop_fn((input,), sequence_length, state, filter_len)
)


u, a, a_m, x_hat_flat = u_ta.stack(), a_ta.stack(), a_m_ta.stack(), x_hat_flat_ta.stack()

x_hat = tf.reshape(x_hat_flat, (seq_size, batch_size, filter_len, input_size))

####

x_hat_f = transform_output(x_hat_flat)

if False:
    reshape_batch = 100
    x_hat_var = tf.Variable(tf.zeros((seq_size, batch_size, input_size), dtype=tf.float32))
    x_hat_f = x_hat_var
    for rbi in xrange(1, seq_size, reshape_batch):
        fut_border = max(seq_size, rbi+reshape_batch)-seq_size
        x_hat_slice = tf.slice(
            x_hat, 
            [rbi, 0, 0, 0], 
            [reshape_batch-fut_border, batch_size, filter_len, input_size]
        )

        for ti, xx in enumerate(tf.unstack(x_hat_slice)):
            ti = rbi + ti
            left_ti = max(0, ti-filter_len)
            
            xx_sliced = tf.slice(xx, (0,0,0), (batch_size, ti-left_ti, input_size))
            xx_sliced = tf.transpose(xx_sliced, (1, 0, 2))/(c.tau*2)
            
            x_hat_f = tf.scatter_add(x_hat_f, range(left_ti, ti), xx_sliced)
        

# gg = tf.gradients(x_hat_f, [net._cells[0].F_flat])

# x_hat_f, _ = tf.tuple(
#     [tf.identity(x_hat_f), tf.variables_initializer([x_hat_var])]
# )

# tf.no_op(tf.variables_initializer([x_hat_var]))

optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)

grads_and_vars = []
for li, s in enumerate(finstate):
    dF = s[-1]
    grads_and_vars += [
        (-tf.reduce_mean(dF, 0), net._cells[li].F_flat),
    ]


apply_grads_step = tf.group(
    optimizer.apply_gradients(grads_and_vars),
    normalize_weights(net)
)



sess = tf.Session()
saver = tf.train.Saver()

env = Env("lca_poc")


model_fname = env.run("model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())


env.clear_pics(env.run())

x_v = np.zeros((seq_size, batch_size, input_size))

for bi in xrange(batch_size):
    for ni in xrange(input_size):
        x_v[:,bi,ni] = generate_ts(seq_size)

# x_orig = generate_ts(seq_size)

# T = np.linspace(filter_len*dt, seq_size*dt - 2*filter_len, int((seq_size-2*filter_len)/dt))
# x_orig = np.pad(np.sin(T/10.0), (filter_len, filter_len), 'constant')


# x_v = x_orig.copy()
# x_v, Ww = white_ts(x_v, filter_len)

# c.lam = 0.05
# x_v = np.concatenate([np.asarray([0.0]), np.diff(x_v)])
# # x_v = np.pad(x_v, (0, 1), 'constant')

x_v = x_v.reshape((seq_size, batch_size, input_size))

# l2_norm = lambda x: np.sqrt(np.sum(np.square(x), 0))
# x_v_n = np.asarray([l2_norm(x_v[(ti-filter_len):ti]) for ti in xrange(filter_len, x_v.shape[0])])
# x_v[filter_len:-filter_len] = x_v[filter_len:-filter_len]/x_v_n[(filter_len/2):-(filter_len/2)]


sess.run(tf.group(*[tf.assign(cell.F_flat, tf.nn.l2_normalize(cell.F_flat, 0)) for cell in net._cells]))

for e in xrange(epochs):
    start_time = time.time()
    state_v = get_zero_state()
    
    u_v, a_v, a_m_v, x_hat_v, x_hat_f_v, finstate_v, F_v, _ = sess.run(
        (
            u, 
            a, 
            a_m, 
            x_hat, 
            x_hat_f, 
            finstate, 
            net._cells[0].F_flat, 
            apply_grads_step, 
        ), 
        {
            input: x_v,
            state: state_v,
            sequence_length: np.asarray([seq_size]*batch_size)
        }
    )

    # sess.run(tf.variables_initializer([x_hat_var]))

    end_time = time.time()
    
    x_hat_f_v2 = np.zeros((seq_size, batch_size, input_size))
    for ti in xrange(x_hat_v.shape[0]):
        left_ti = max(0, ti-filter_len)
        x_hat_f_v2[left_ti:ti] += np.transpose(x_hat_v[ti,:, :(ti-left_ti), :], (1, 0, 2))/(c.tau * 2)

    # error = np.mean(np.square(x_hat_f_v[filter_len:-filter_len] - x_v[filter_len:-filter_len]))
    error = 0.0
    print "Epoch {} ({}), MSE {:}".format(
        e, 
        round(end_time-start_time, 3), 
        error
    )



# a_m_v = np.zeros((seq_size, batch_size, layer_size))

# a_m = np.zeros((batch_size, layer_size))
# for ti in xrange(seq_size):
# 	a_m = (1.0 - 1.0/c.tau_m) * a_m + (1.0/c.tau_m) * a_v[ti]
	
# 	a_m_v[ti] = a_m.copy()


# shl(x_hat_f_v, x_v, show=True)
# shl(a_v[:500])
# shm(a_v[0:500,0,:])
