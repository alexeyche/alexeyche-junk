#!/usr/bin/env python

import os
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob
from env import Env

import collections


from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell as rc
from tensorflow.python.ops import math_ops as mo
from tensorflow.python.ops import array_ops

def sm(matrix):
    plt.imshow(np.squeeze(matrix).T)
    plt.colorbar()
    plt.show()

def smooth(signal, sigma=0.01, filter_size=50):
    lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
    return np.convolve(lf_filter, signal, mode="same")

def smooth_matrix(m, sigma=0.01, filter_size=50):
    res = np.zeros(m.shape)
    for dim_idx in xrange(m.shape[1]):
        res[:, dim_idx] = smooth(m[:, dim_idx], sigma, filter_size)
    return res

seq_size = 100

factor = 0.1
dt = 1.0/1000.0
dt_ms = dt * 1000.0

lrate = 1e-03

batch_size = 1
input_size = 100
hidden_size = 100
visible_size = 10
net_size = hidden_size + visible_size

tau_syn = dt_ms/10.0
tau_mem = dt_ms/10.0
tau_refr = dt_ms/5.0
tau_learn = dt_ms/10.0
tau_long = dt_ms/float(seq_size)
amp_refr = 100.0

env = Env("simple_test")
_GLMInputTuple = collections.namedtuple("GLMInputTuple", ("input", "target"))
_GLMStateTuple = collections.namedtuple("GLMStateTuple", ("u", "s", "r", "spikes", "dW", "reward", "reward_mean"))
_GLMOutputTuple = collections.namedtuple("GLMOutputTuple", ("spikes", "a", "factor", "reward", "reward_mean"))

class GLMInputTuple(_GLMInputTuple):
    __slots__ = ()


class GLMStateTuple(_GLMStateTuple):
    __slots__ = ()

class GLMOutputTuple(_GLMOutputTuple):
    __slots__ = ()


def safe_log(v):
    return tf.log(tf.maximum(v, 1e-08))

def exp_act(u, gain=0.1):
    return tf.exp(u) * gain

def sigmoid_act(u, u_rest=-5.0):
    return tf.sigmoid(u_rest + u)

def log_exp_act(u, threshold=0.0, slope=1.0):
    return 0.1 * (tf.log(1.0 + tf.exp((u-threshold)/slope))  - tf.log(1.0 + tf.exp(-threshold/slope)))

def outer(left_v, right_v):
    return mo.matmul(tf.expand_dims(left_v, 1), tf.expand_dims(right_v, 0))


class GLMCell(rc.RNNCell):
    def __init__(self, input_size, hidden_size, visible_size, activation = sigmoid_act):
        self._activation = activation
        self._num_units = visible_size + hidden_size
        self._input_size = input_size
        self._visible_size = visible_size
        self._hidden_size = hidden_size

        self.W = None
        self._state_is_tuple = True
        
    @property
    def state_size(self):
        return GLMStateTuple(
            self._num_units, 
            self._input_size + self._num_units, 
            self._num_units, 
            self._num_units, 
            (self._input_size, self._num_units),
            self._num_units,
            self._num_units
        )

    @property
    def output_size(self):
        return GLMOutputTuple(self._num_units, self._num_units, self._num_units, 1, 1)


    def _init_parameters(self):
        if self.W is None:
            self.W = vs.get_variable("W", [self._input_size + self._num_units, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))
        

    def slice(self, t):
        return (
            tf.slice(t, [0, 0], [batch_size, self._hidden_size]),  
            tf.slice(t, [0, self._hidden_size], [batch_size, self._visible_size])
        )

    def __call__(self, input_tuple, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            self._init_parameters()

            _input, target = input_tuple
            u, s, r, inner_spikes, dW, reward, reward_mean = state
            
            s = (1.0 - tau_syn) * s + tf.concat(1, [_input, inner_spikes])
            u = (1.0 - tau_mem) * u + mo.matmul(s, self.W)
            r = (1.0 - tau_refr) * r
            

            act_raw = self._activation(u)
            act = act_raw * tf.exp(-r)

            spikes = tf.select(
                act > tf.random_uniform([batch_size, self._num_units]),
                tf.ones([batch_size, self._num_units]),
                tf.zeros([batch_size, self._num_units])
            )   
            
            hidden_spikes, _ = self.slice(spikes)
            _, act_visible = self.slice(act)
            
            
            reward_mean = (1.0 - tau_long) * reward_mean + tau_long * reward
            reward = (1.0 - tau_learn) * reward + tau_learn * tf.reduce_sum(
                target * safe_log(act_visible*dt) + (1.0 - target) * safe_log(1.0 - act_visible*dt)
            , 1)
            
            r += spikes * amp_refr

            act_grad = tf.gradients([act], [u])[0]
            
            learn_target = tf.concat(1, [hidden_spikes, target])

            factor = tf.concat(1, [
                (reward - reward_mean) * tf.ones((batch_size, hidden_size,)), 
                tf.ones((batch_size, visible_size,))
            ])

            dW += lrate * outer(
                tf.reduce_sum(s, 0),
                tf.reduce_sum( (act_grad/act_raw) * (learn_target - act) * factor, 0) 
            )

            return GLMOutputTuple(spikes, act, factor, reward, reward_mean), GLMStateTuple(u, s, r, spikes, dW, reward, reward_mean)


cell_type = GLMCell

cell = cell_type(input_size, hidden_size, visible_size)


input = GLMInputTuple(
    tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input"),
    tf.placeholder(tf.float32, shape=(seq_size, batch_size, visible_size), name="Target"),
)


state = GLMStateTuple(
    tf.placeholder(tf.float32, [batch_size, net_size], name="u"),
    tf.placeholder(tf.float32, [batch_size, input_size + net_size], name="s"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="r"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="spikes"),
    tf.placeholder(tf.float32, [input_size + net_size, net_size], name="dW"),
    tf.placeholder(tf.float32, [batch_size, 1], name="reward"),
    tf.placeholder(tf.float32, [batch_size, 1], name="reward_mean"),
)

net_out, finstate = rnn.dynamic_rnn(cell, input, initial_state=state, time_major=True)
spikes, a, factor, reward, reward_mean = net_out

_, a_visible = tf.slice(a, [0, 0, 0], [seq_size, batch_size, hidden_size]), tf.slice(a, [0, 0, hidden_size], [seq_size, batch_size, visible_size])

log_ll = tf.reduce_mean(input.target * safe_log(a_visible*dt) + (1.0 - input.target) * safe_log(1.0 - a_visible*dt))

## RUNNING

inputs_v = np.zeros((seq_size, batch_size, input_size))

x_rate = 2.0
for bi in xrange(batch_size):
    for tt in xrange(seq_size):
        for ni in xrange(input_size):
            if np.random.random() <= x_rate/seq_size:
                inputs_v[tt, bi, ni] = 1.0


# for bi in xrange(batch_size):
#     for si in xrange(seq_size):
#         if si % 5 == 0:
#             inputs_v[si, bi, int((float(si)/seq_size) * input_size)] = 1.0


targets_v = np.zeros((seq_size, batch_size, visible_size))

for bi in xrange(batch_size):
    for si in xrange(seq_size):
        if si % visible_size == 0:
            targets_v[si, bi, int((float(si)/seq_size) * visible_size)] = 1.0

# targets_v[seq_size/2, 0, 2] = 1.0

sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("glm_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.initialize_all_variables())


target_smooth = smooth_matrix(np.squeeze(targets_v))

writer = tf.train.SummaryWriter(env.run("summary"), sess.graph)

reward_v, reward_mean_v = None, None

epochs = 1000
for e in xrange(epochs):
    state_v = GLMStateTuple(
        np.zeros((batch_size, net_size)),
        np.zeros((batch_size, input_size + net_size)),
        np.zeros((batch_size, net_size)),
        np.zeros((batch_size, net_size)),
        np.zeros((input_size + net_size, net_size)),
        (reward_v[-1,:,:] if not reward_v is None else np.zeros((batch_size, 1))),
        (reward_mean_v[-1,:,:] if not reward_mean_v is None else np.zeros((batch_size, 1))),
    )

    sess_out = sess.run(
        [
            a,
            spikes,
            finstate,
            log_ll,
            factor,
            reward,
            reward_mean,
        ],
        {
            input: GLMInputTuple(inputs_v, targets_v),
            state: state_v,
        }
    )

    a_v, spikes_v, finstate_v, log_ll_v, factor_v, reward_v, reward_mean_v = sess_out
    
    if hidden_size > 0:
        output_smooth = smooth_matrix(np.squeeze(spikes_v[:, :, hidden_size:]))
    else:
        output_smooth = smooth_matrix(np.squeeze(spikes_v))
    
    loss = np.mean(np.square(output_smooth - target_smooth))
    dW = finstate_v.dW
    assert not np.any(np.isnan(dW)), "Found nan"

    old_W = sess.run(cell.W)
    sess.run(cell.W.assign(cell.W + dW))
    new_W = sess.run(cell.W)

    print "Epoch {}, log ll {}, loss {}".format(e, log_ll_v, loss)