import os
import time
import tensorflow as tf
from matplotlib.pyplot import cm

from hist_rnn import rnn_with_hist_loop_fn
from config import Config
from env import Env

from model import LCACell, normalize_weights, exp_poisson
from tensorflow.contrib.rnn import MultiRNNCell

from ts_pp import white_ts, generate_ts

from util import *


lrate = 0.01
epochs = 1

tf.set_random_seed(1)
np.random.seed(1)

input_size = 1

seq_size = 300
batch_size = 1
layer_size = 2
filter_len = 1


dt = 1.0

c = Config()
c.lam = 0.01
c.weight_init_factor = 0.5
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0/seq_size
c.simple_hebb = True
c.tau_m = 10.0
c.adapt = 5.0
c.act_factor = 1.0
c.adaptive = True

c.act_factor = tf.placeholder(tf.float32, shape=(), name="act_factor")
c.adapt = tf.placeholder(tf.float32, shape=(), name="adapt")
c.tau_m = tf.placeholder(tf.float32, shape=(), name="tau_m")


input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")
sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)

Finput = tf.placeholder(tf.float32, shape=(filter_len * input_size, layer_size), name="Finput")


net = MultiRNNCell([
    LCACell(input_size, layer_size, filter_len, c, tf.nn.relu, Finput = Finput),
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
    rnn_with_hist_loop_fn(input, sequence_length, state, filter_len)
)


u, a, a_m, x_hat_flat = u_ta.stack(), a_ta.stack(), a_m_ta.stack(), x_hat_flat_ta.stack()

x_hat = tf.reshape(x_hat_flat, (seq_size, batch_size, filter_len, input_size))

        
grads_and_vars = []
for li, s in enumerate(finstate):
    dF = s[-1]
    grads_and_vars += [
        (-tf.reduce_mean(dF, 0), net._cells[li].F_flat),
    ]


sess = tf.Session()

env = Env("lca_simple")


x_v = np.zeros((seq_size, batch_size, input_size))

for bi in xrange(batch_size):
    for ni in xrange(input_size):
        x_v[:,bi,ni] = generate_ts(seq_size)

x_v = x_v.reshape((seq_size, batch_size, input_size))

state_v = get_zero_state()

sample_size = 20

E = np.zeros((sample_size, sample_size))

a_v_res = np.zeros((sample_size, sample_size, seq_size, layer_size))
x_v_res = np.zeros((sample_size, sample_size, seq_size))

W0 = np.linspace(-2.0, 2.0, sample_size)
W1 = np.linspace(-2.0, 2.0, sample_size)


plot_type = 2

if plot_type == 0:
    
    dW0res = np.zeros((sample_size, sample_size))
    dW1res = np.zeros((sample_size, sample_size))

    for w0_idx, w0 in enumerate(W0):
        for w1_idx, w1 in enumerate(W1):
            w_v = np.asarray([[w0, w1]])

            u_v, a_v, a_m_v, x_hat_v, finstate_v, F_v, g_and_v = sess.run(
    	        (
    	            u, 
    	            a, 
    	            a_m, 
    	            x_hat, 
    	            finstate, 
    	            net._cells[0].F_flat, 
    	            grads_and_vars
    	        ), 
    	        {
    	            input: x_v,
    	            Finput: w_v,
    	            state: state_v,
    	            sequence_length: np.asarray([seq_size]*batch_size),
                    c.act_factor: 1.0,
                    c.adapt: 2.0,
                    c.tau_m: 50.0
    	        }
    	    )

            x_hat_f_v = np.zeros((seq_size, batch_size, input_size))
            for ti in xrange(x_hat_v.shape[0]):
                left_ti = max(0, ti-filter_len)
                x_hat_f_v[left_ti:ti] += np.transpose(x_hat_v[ti,:, :(ti-left_ti), :], (1, 0, 2))

            error = np.mean(np.square(x_hat_f_v[filter_len:-filter_len] - x_v[filter_len:-filter_len]))
            E[w0_idx, w1_idx] = error

            dW0res[w0_idx, w1_idx] = g_and_v[0][0][0,0]
            dW1res[w0_idx, w1_idx] = g_and_v[0][0][0,1]

            a_v_res[w0_idx, w1_idx] = a_v[:,0,:].copy()
            x_v_res[w0_idx, w1_idx] = x_hat_f_v[:,0,0].copy()


    plot = plt.figure(figsize=(20,10))
    plt.quiver(
        W0, W1, dW0res, dW1res,
        E,
        cmap=cm.seismic
    )
    plt.colorbar()
    plt.show()

elif plot_type == 1:
    adapt_vec = np.linspace(0.01, 5.0, sample_size)
    act_factor_vec = np.linspace(1.0, 1.0, sample_size)
    
    dAdres0 = np.zeros((sample_size,sample_size))
    dAdres1 = np.zeros((sample_size,sample_size))
    
    w_v = np.asarray([[W0[-1], W1[0]]])
    for act_factor_id, act_factor in enumerate(act_factor_vec):
        for adapt_id, adapt in enumerate(adapt_vec):
            u_v, a_v, a_m_v, x_hat_v, finstate_v, F_v, g_and_v = sess.run(
                (
                    u, 
                    a, 
                    a_m, 
                    x_hat, 
                    finstate, 
                    net._cells[0].F_flat, 
                    grads_and_vars
                ), 
                {
                    input: x_v,
                    Finput: w_v,
                    state: state_v,
                    sequence_length: np.asarray([seq_size]*batch_size),
                    c.act_factor: act_factor,
                    c.adapt: adapt,
                    c.tau_m: 10.0
                }
            )

            x_hat_f_v = np.zeros((seq_size, batch_size, input_size))
            for ti in xrange(x_hat_v.shape[0]):
                left_ti = max(0, ti-filter_len)
                x_hat_f_v[left_ti:ti] += np.transpose(x_hat_v[ti,:, :(ti-left_ti), :], (1, 0, 2))

            error = np.mean(np.square(x_hat_f_v[filter_len:-filter_len] - x_v[filter_len:-filter_len]))
            E[act_factor_id, adapt_id] = error

            dAdres0[act_factor_id, adapt_id] = g_and_v[0][0][0,0]
            dAdres1[act_factor_id, adapt_id] = g_and_v[0][0][0,1]

            a_v_res[act_factor_id, adapt_id] = a_v[:,0,:].copy()
            x_v_res[act_factor_id, adapt_id] = x_hat_f_v[:,0,0].copy()
    shm(E)

elif plot_type == 2:
    tau_m_vec = np.linspace(1.0, 200.0, sample_size)
    adapt_vec = np.linspace(0.01, 10.0, sample_size)
    
    dAdres0 = np.zeros((sample_size,sample_size))
    dAdres1 = np.zeros((sample_size,sample_size))
    
    w_v = np.asarray([[W0[-1], W1[0]]])
    for tau_m_id, tau_m in enumerate(tau_m_vec):
        for adapt_id, adapt in enumerate(adapt_vec):
            u_v, a_v, a_m_v, x_hat_v, finstate_v, F_v, g_and_v = sess.run(
                (
                    u, 
                    a, 
                    a_m, 
                    x_hat, 
                    finstate, 
                    net._cells[0].F_flat, 
                    grads_and_vars
                ), 
                {
                    input: x_v,
                    Finput: w_v,
                    state: state_v,
                    sequence_length: np.asarray([seq_size]*batch_size),
                    c.act_factor: 1.0,
                    c.adapt: adapt,
                    c.tau_m: tau_m
                }
            )

            x_hat_f_v = np.zeros((seq_size, batch_size, input_size))
            for ti in xrange(x_hat_v.shape[0]):
                left_ti = max(0, ti-filter_len)
                x_hat_f_v[left_ti:ti] += np.transpose(x_hat_v[ti,:, :(ti-left_ti), :], (1, 0, 2))

            error = np.mean(np.square(x_hat_f_v[filter_len:-filter_len] - x_v[filter_len:-filter_len]))
            E[tau_m_id, adapt_id] = error

            dAdres0[tau_m_id, adapt_id] = g_and_v[0][0][0,0]
            dAdres1[tau_m_id, adapt_id] = g_and_v[0][0][0,1]

            a_v_res[tau_m_id, adapt_id] = a_v[:,0,:].copy()
            x_v_res[tau_m_id, adapt_id] = x_hat_f_v[:,0,0].copy()
    shm(E)