import os

import tensorflow as tf
import pandas as pd
from functools import partial
import time
from util import *

from hist_rnn import rnn_with_hist_loop_fn
from config import Config
from env import Env

from model import LCACell, normalize_weights, exp_poisson, poisson, LCAScalarCell
from tensorflow.contrib.rnn import MultiRNNCell

import scikits.statsmodels.tsa.api as smt
from scikits.statsmodels.tsa.arima_process import arma_generate_sample

from ts_pp import white_ts, generate_ts

from util import *


lrate = 0.01
# lrate *= 100.0

epochs = 500

seed = 5
tf.set_random_seed(seed)
np.random.seed(seed)

input_size = 50
seq_size = 300
batch_size = 1
layer_size = 25
filter_len = 10

c = Config()

c.weight_init_factor = 1.0
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0

c.lam = 0.025
c.adaptive_threshold = False

c.tau_m = 50.0
c.adapt = 5.0
c.act_factor = 1.0
c.adaptive = False

c.tau_fb = 10.0
c.fb_factor = 2.0
c.smooth_feedback = False


hand_grad = False

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")
sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)

act = tf.nn.relu
# act = lambda x: poisson(tf.nn.relu(x))

net = MultiRNNCell([
    # LCAScalarCell(input_size, layer_size, c, act=act),
    LCACell(input_size, layer_size, filter_len, c=c, act=act),
])

state = tuple(
    tuple((
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="u"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a_m"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="fb_m"),
        tf.placeholder(tf.float32, [batch_size, filter_len * cell.input_size, cell.layer_size], name="dF"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size, cell.layer_size], name="dFc"),
    )) for cell in net._cells
)

get_zero_state = lambda: tuple(
    tuple(
        np.zeros((batch_size,) + tuple(t.get_shape().as_list()[1:])) 
        for t in tup
    )
    for tup in state
)


# (a, u, a_m, x_hat, fb), finstate = tf.nn.dynamic_rnn(net, (input, ), initial_state=state, time_major=True)

(a_ta, u_ta, a_m_ta, x_hat_flat_ta, fb_m_ta), finstate, _ = tf.nn.raw_rnn(
    net, 
    rnn_with_hist_loop_fn((input,), sequence_length, state, filter_len)
)

(
    a, 
    u,
    a_m, 
    x_hat_flat, 
    fb
) = (
    a_ta.stack(), 
    u_ta.stack(), 
    a_m_ta.stack(), 
    x_hat_flat_ta.stack(), 
    fb_m_ta.stack()
)

x_hat = tf.reshape(x_hat_flat, (seq_size, batch_size, filter_len, input_size))

if isinstance(net._cells[0], LCAScalarCell):
    loss = tf.nn.l2_loss(input - x_hat)

    dF_grad_base, dFc_grad_base = tf.gradients(loss, [net._cells[0]._params[0], net._cells[0]._params[1]])


optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)

hand_grad = hand_grad or isinstance(net._cells[0], LCACell)

grads_and_vars = []
if hand_grad:
    for li, s in enumerate(finstate):
        dF = s[-2]
        dFc = s[-1]
        
        F = net._cells[li].F_flat if isinstance(net._cells[li], LCACell) else net._cells[li].F

        grads_and_vars += [
            (tf.reduce_sum(dF, 0), F),
            (tf.reduce_sum(dFc, 0), net._cells[li].Fc),
        ]
else:
    grads_and_vars = [
        (dF_grad_base, net._cells[0].F),
        (dFc_grad_base, net._cells[0].Fc)
    ]
    

apply_grads_step = tf.group(
    optimizer.apply_gradients(grads_and_vars),
    # normalize_weights(net)
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
x_v_sm = np.zeros((seq_size, batch_size, input_size))

for bi in xrange(batch_size):
    # for si in xrange(0, seq_size, 5):
        # x_v[si, bi, si % input_size] = 1.0
    for si in xrange(seq_size):
        for ni in xrange(input_size):
            if np.random.random() < 0.005:
                x_v[si, bi, ni] = 1.0
    x_v_sm[:, bi, :] = smooth_matrix(x_v[:, bi, :])


# sess.run(tf.group(*[tf.assign(cell.F_flat, tf.nn.l2_normalize(cell.F_flat, 0)) for cell in net._cells]))
# sess.run(tf.group(*[tf.assign(cell.Fc, tf.matmul(tf.transpose(cell.F_flat), cell.F_flat) - tf.eye(cell.F_flat.get_shape()[1].value)) for cell in net._cells]))

# sess.run(tf.group(*[tf.assign(cell.Fc, tf.matrix_set_diag(tf.matmul(tf.transpose(cell.F_flat), cell.F_flat), tf.zeros(cell.F_flat.get_shape()[1].value))) for cell in net._cells]))
try:
    for e in xrange(epochs):
        start_time = time.time()
        state_v = get_zero_state()
        
        u_v, a_v, a_m_v, x_hat_v, finstate_v, F_v, fb_v, _ = sess.run(
            (
                u, 
                a, 
                a_m, 
                x_hat, 
                finstate, 
                net._cells[0].F, 
                fb,
                apply_grads_step, 
            ), 
            {
                input: x_v,
                state: state_v,
                sequence_length: np.asarray([seq_size]*batch_size)
           }
        )

        dF_grad_new = np.sum(finstate_v[0][-2], 0)
        dFc_grad_new = np.sum(finstate_v[0][-1], 0)

        # sess.run(tf.variables_initializer([x_hat_var]))
        x_hat_f_v2 = np.zeros((seq_size, batch_size, input_size))
        for ti in xrange(x_hat_v.shape[0]):
            left_ti = max(0, ti-filter_len)
            x_hat_f_v2[left_ti:ti] += np.transpose(x_hat_v[ti,:, :(ti-left_ti), :], (1, 0, 2))/c.tau

        end_time = time.time()
        
        x_hat_f_v2_sm = x_hat_f_v2.copy()
        for bi in xrange(batch_size):
            x_hat_f_v2_sm[:, bi, :] = smooth_matrix(x_hat_f_v2[:, bi, :])

        error = np.mean(np.square(x_hat_f_v2_sm[filter_len:-filter_len] - x_v_sm[filter_len:-filter_len]))
        
        print "Epoch {} ({}), MSE {:}".format(
            e, 
            round(end_time-start_time, 3), 
            error
        )
except KeyboardInterrupt:
    pass

# shl(a_v[:500, 0, :])
shm(a_v[100:500,0,:])
shm(x_hat_f_v2[:,0,:], x_v[:,0,:])


x2_v = np.zeros((seq_size, batch_size, input_size))
x2_v_sm = np.zeros((seq_size, batch_size, input_size))

for bi in xrange(batch_size):
    # for si in xrange(0, seq_size, 5):
        # x_v[si, bi, si % input_size] = 1.0
    for si in xrange(seq_size):
        for ni in xrange(input_size):
            if np.random.random() < 0.005:
                x2_v[si, bi, ni] = 1.0
    x2_v_sm[:, bi, :] = smooth_matrix(x2_v[:, bi, :])

a2_v, x2_hat_v = sess.run(
    [a, x_hat], 
    {
        input: x2_v, 
        state: get_zero_state(), 
        sequence_length: np.asarray([seq_size]*batch_size)
    }
)

x2_hat_f_v = np.zeros((seq_size, batch_size, input_size))
for ti in xrange(x2_hat_v.shape[0]):
    left_ti = max(0, ti-filter_len)
    x2_hat_f_v[left_ti:ti] += np.transpose(x2_hat_v[ti,:, :(ti-left_ti), :], (1, 0, 2))/c.tau

x2_hat_f_v_sm = x2_hat_f_v.copy()
for bi in xrange(batch_size):
    x2_hat_f_v_sm[:, bi, :] = smooth_matrix(x2_hat_f_v[:, bi, :])

shm(x2_hat_f_v[:,0,:], x2_v[:,0,:])
# error = np.mean(np.square(x2_hat_f_v_sm[filter_len:-filter_len] - x2_v_sm[filter_len:-filter_len]))
error_line = np.sum(np.square(x_hat_f_v2_sm[filter_len:-filter_len] - x_v_sm[filter_len:-filter_len]), 2)
error_line2 = np.sum(np.square(x2_hat_f_v_sm[filter_len:-filter_len] - x2_v_sm[filter_len:-filter_len]), 2)
shl(error_line, error_line2)

