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

epochs = 1

tf.set_random_seed(1)
np.random.seed(1)

input_size = 1
seq_size = 300
batch_size = 1
layer_size = 2

c = Config()
c.lam = 0.15
c.weight_init_factor = 0.5
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0/seq_size
c.simple_hebb = True
c.tau_m = 50.0
c.adapt = 5.0
c.act_factor = 1.0
c.adaptive = False
c.tau_fb = 50.0

new_grad = False

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")
Finput = tf.placeholder(tf.float32, shape=(input_size, layer_size), name="Finput")

act = tf.nn.relu

net = MultiRNNCell([
    LCAScalarCell(input_size, layer_size, c, act=act, Finput=Finput),
])

state = tuple(
    tuple((
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="u"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a_m"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="fb_m"),
        tf.placeholder(tf.float32, [batch_size, cell.input_size, cell.layer_size], name="dF"),
    )) for cell in net._cells
)

get_zero_state = lambda: tuple(
    np.zeros((batch_size,) + tuple(t.get_shape().as_list()[1:]))
    for tup in state for t in tup
)


(u, a, a_m, x_hat, fb), finstate = tf.nn.dynamic_rnn(net, (input, ), initial_state=state, time_major=True)


sess = tf.Session()

x_v = np.zeros((seq_size, batch_size, input_size))

for bi in xrange(batch_size):
    for ni in xrange(input_size):
        x_v[:,bi,ni] = generate_ts(seq_size)

x_v = x_v.reshape((seq_size, batch_size, input_size))

sample_size = 30

W0 = np.linspace(-10.0, 10.0, sample_size)
W1 = np.linspace(-10.0, 10.0, sample_size)


std = np.zeros((sample_size, sample_size))
a_v_res = np.zeros((sample_size, sample_size, seq_size, layer_size))
fb_v_res = np.zeros((sample_size, sample_size, seq_size, layer_size))

for w0_idx, w0 in enumerate(W0):
    for w1_idx, w1 in enumerate(W1):

        state_v = get_zero_state()
        
        F_v = np.asarray([[w0, w1]])
        # F_v = sess.run(tf.nn.l2_normalize(F_v, 0))

        u_v, a_v, a_m_v, x_hat_v, finstate_v, fb_v = sess.run(
            (
                u, 
                a, 
                a_m, 
                x_hat, 
                finstate, 
                fb
            ), 
            {
                input: x_v,
                state: state_v,
                Finput: F_v
            }
        )

        std[w0_idx, w1_idx] = np.std(a_v)
        a_v_res[w0_idx, w1_idx] = a_v[:,0,:].copy()
        fb_v_res[w0_idx, w1_idx] = fb_v[:,0,:].copy()