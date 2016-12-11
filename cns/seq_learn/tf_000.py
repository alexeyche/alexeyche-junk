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

N = 1
lrate = 1e-02

batch_size = 1
input_size = 100
net_size = 10
layer_dims = [net_size, net_size]
num_of_layers = len(layer_dims)


tau_syn = dt_ms/10.0
tau_mem = dt_ms/10.0
tau_refr = dt_ms/2.0
tau_learn = dt_ms/100.0
amp_refr = 50.0

env = Env("simple_test")
_GLMStateTuple = collections.namedtuple("GLMStateTuple", ("u", "s", "r", "spikes", "dW", "log_ll"))
_GLMOutputTuple = collections.namedtuple("GLMOutputTuple", ("input", "target", "a"))

class GLMStateTuple(_GLMStateTuple):
    __slots__ = ()

class GLMOutputTuple(_GLMOutputTuple):
    __slots__ = ()


def safe_log(v):
    return tf.log(tf.maximum(v, 1e-08))

def exp_act(u, gain=0.1):
    return tf.exp(u) * gain

def sigmoid_act(u):
    return tf.sigmoid(u)

def log_exp(u, threshold=0.0, slope=1.0):
    return dt *(tf.log(1.0 + tf.exp((u-threshold)/slope)) - tf.log(1.0 + tf.exp(-threshold/slope)))

def outer(left_v, right_v):
    return mo.matmul(tf.expand_dims(left_v, 1), tf.expand_dims(right_v, 0))

class GLMCell(rc.RNNCell):
    def __init__(self, input_size, num_units, top_layer, global_signal, activation = exp_act):
        self._num_units = num_units
        self._activation = activation
        self._input_size = input_size
        self._top_layer = top_layer
        self._global_signal = global_signal

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
            self._num_units
        )

    @property
    def output_size(self):
        return GLMOutputTuple(self._num_units, self._num_units, self._num_units)


    def _init_parameters(self):
        if self.W is None:
            self.W = vs.get_variable("W", [self._input_size + self._num_units, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))
        

    def __call__(self, input_tuple, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            self._init_parameters()

            _input, target, _ = input_tuple
            u, s, r, inner_spikes, dW, log_ll = state

            s = (1.0 - tau_syn) * s + tf.concat(1, [_input, inner_spikes])
            u = (1.0 - tau_mem) * u + mo.matmul(s, self.W)
            r = (1.0 - tau_refr) * r
            

            a_raw = self._activation(u)
            a = a_raw * tf.exp(-r)

            spikes = tf.select(
                a > tf.random_uniform([batch_size, self._num_units]),
                tf.ones([batch_size, self._num_units]),
                tf.zeros([batch_size, self._num_units])
            )   
            if self._top_layer:
                target_spikes = target
            else:
                target_spikes = spikes
            
            
            log_ll = (1.0 - tau_learn) * log_ll + tau_learn * (
                target_spikes * safe_log(a*dt) + (1.0 - target_spikes) * safe_log(1.0 - a*dt)
            )
            
            if self._top_layer:
                self._global_signal += tf.reduce_mean(log_ll)

            r += spikes * amp_refr

            a_grad = tf.gradients([a], [u])[0]

            dW += lrate * outer(
                tf.reduce_sum(s, 0),
                tf.reduce_sum( (a_grad/a_raw) * (target_spikes - a), 0)
            ) * (1.0 if self._top_layer else self._global_signal)

            return GLMOutputTuple(spikes, target, a), GLMStateTuple(u, s, r, spikes, dW, log_ll)



cell_type = GLMCell

global_signal = tf.Variable(initial_value=0.0, trainable=False, name="global_signal")

all_dims = [input_size] + layer_dims
cells_list = []
for di in xrange(1, len(all_dims)):
    prev_dim = all_dims[di-1]
    next_dim = all_dims[di]

    cells_list.append(
        cell_type(prev_dim, next_dim, di == len(all_dims) - 1, global_signal)
    )

cells = rc.MultiRNNCell(cells_list, state_is_tuple=True)

input = GLMOutputTuple(
    tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input"),
    tf.placeholder(tf.float32, shape=(seq_size, batch_size, layer_dims[0]), name="Target"),
    tf.placeholder(tf.float32, shape=(seq_size, batch_size, layer_dims[0]), name="Activation")
)

state_tuples = []
for di in xrange(1, len(all_dims)):
    prev_dim = all_dims[di-1]
    next_dim = all_dims[di]

    state_tuples.append(
        GLMStateTuple(
            tf.placeholder(tf.float32, [batch_size, next_dim], name="u{}".format(di)),
            tf.placeholder(tf.float32, [batch_size, prev_dim + next_dim], name="s{}".format(di)),
            tf.placeholder(tf.float32, [batch_size, next_dim], name="r{}".format(di)),
            tf.placeholder(tf.float32, [batch_size, next_dim], name="spikes{}".format(di)),
            tf.placeholder(tf.float32, [prev_dim + next_dim, next_dim], name="dW{}".format(di)),
            tf.placeholder(tf.float32, [batch_size, next_dim], name="log_ll{}".format(di)),
        )
    )

state = tuple(state_tuples)

net_out, finstate = rnn.dynamic_rnn(cells, input, initial_state=state, time_major=True)
spikes, target, a = net_out

log_ll = tf.reduce_mean(input.target * safe_log(a*dt) + (1.0 - input.target) * safe_log(1.0 - a*dt))

## RUNNING
state_tuples_v = []
for di in xrange(1, len(all_dims)):
    prev_dim = all_dims[di-1]
    next_dim = all_dims[di]

    state_tuples_v.append(
        GLMStateTuple(
            np.zeros((batch_size, next_dim)),
            np.zeros((batch_size, prev_dim + next_dim)),
            np.zeros((batch_size, next_dim)),
            np.zeros((batch_size, next_dim)),
            np.zeros((prev_dim + next_dim, next_dim)),
            np.zeros((batch_size, next_dim)),
        )
    )

state_v = tuple(state_tuples_v)

inputs_v = np.zeros((seq_size, batch_size, input_size))

x_rate = 2.0
for bi in xrange(batch_size):
    for tt in xrange(seq_size):
        for ni in xrange(input_size):
            if np.random.random() <= x_rate/seq_size:
                inputs_v[tt, bi, ni] = 1.0


# for bi in xrange(batch_size):
#     for si in xrange(seq_size):
#         if si % net_size == 0:
#             inputs_v[si, bi, int((float(si)/seq_size) * input_size)] = 1.0


targets_v = np.zeros((seq_size, batch_size, net_size))

for bi in xrange(batch_size):
    for si in xrange(seq_size):
        if si % net_size == 0:
            targets_v[si, bi, int((float(si)/seq_size) * net_size)] = 1.0

# targets_v[seq_size/2, 0, 2] = 1.0

a_v_blank = np.zeros((seq_size, batch_size, net_size))

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

epochs = 2
for e in xrange(epochs):
    sess_out = sess.run(
        [
            a,
            spikes,
            finstate,
            log_ll,
            global_signal
        ],
        {
            input: GLMOutputTuple(inputs_v, targets_v, a_v_blank),
            state: state_v,
        }
    )

    a_v, spikes_v, finstate_v, log_ll_v = sess_out

    output_smooth = smooth_matrix(np.squeeze(spikes_v))
    loss = np.mean(np.square(output_smooth - target_smooth))


    for cell, st in zip(cells._cells, finstate_v):
        dW = st.dW
        assert not np.any(np.isnan(dW)), "Found nan"

        old_W = sess.run(cell.W)
        sess.run(cell.W.assign(cell.W + dW))
        new_W = sess.run(cell.W)

    print "Epoch {}, log ll {}, loss {}".format(e, log_ll_v, loss)