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
N = 1
lrate = 1e-02

num_of_layers = 1
batch_size = 1
input_size = 100
net_size = 10

tau_syn = 1.0/10.0
tau_mem = 1.0/10.0


env = Env("simple_test")

_GLMStateTuple = collections.namedtuple("GLMStateTuple", ("u", "s", "dW"))
_GLMInputTuple = collections.namedtuple("GLMInputTuple", ("input", "target"))
_GLMOutputTuple = collections.namedtuple("GLMOutputTuple", ("a", "spikes"))

class GLMStateTuple(_GLMStateTuple):
    __slots__ = ()

class GLMInputTuple(_GLMInputTuple):
    __slots__ = ()

class GLMOutputTuple(_GLMOutputTuple):
    __slots__ = ()


def outer(left_v, right_v):
    return mo.matmul(tf.expand_dims(left_v, 1), tf.expand_dims(right_v, 0))

class GLMCell(rc.RNNCell):
    def __init__(self, num_units, activation = mo.exp):
        self._num_units = num_units
        self._activation = activation

        self.W = None
        self._state_is_tuple = True

    @property
    def state_size(self):
        return GLMStateTuple(self._num_units, input_size, (input_size, self._num_units))

    @property
    def output_size(self):
        return GLMOutputTuple(self._num_units, self._num_units)


    def _init_parameters(self):
        if self.W is None:
            self.W = vs.get_variable("W", [input_size, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))

    def __call__(self, input_tuple, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            self._init_parameters()

            _input, target = input_tuple
            u, s, dW = state

            s = (1.0 - tau_syn) * s + _input            
            u = (1.0 - tau_mem) * u + mo.matmul(s, self.W)

            a = self._activation(u)

            spikes = tf.select(
                a * dt > tf.random_uniform([batch_size, self._num_units]), 
                tf.ones([batch_size, self._num_units]),
                tf.zeros([batch_size, self._num_units])
            )

            a_grad = tf.gradients([a], [u])[0]

            dW += lrate * outer(
                tf.reduce_sum(s, 0),  
                tf.reduce_sum( (a_grad/a) * (target - dt*a), 0)
            )

            return GLMOutputTuple(a, spikes), GLMStateTuple(u, s, dW)





cell_type = GLMCell

cells = rc.MultiRNNCell([cell_type(net_size) for _ in xrange(num_of_layers)], state_is_tuple=True)

input = GLMInputTuple(
    tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input"),
    tf.placeholder(tf.float32, shape=(seq_size, batch_size, net_size), name="Target")
)

state = tuple([ GLMStateTuple(
    tf.placeholder(tf.float32, [batch_size, net_size], name="u{}".format(li)),
    tf.placeholder(tf.float32, [batch_size, input_size], name="s{}".format(li)),
    tf.placeholder(tf.float32, [input_size, net_size], name="dW{}".format(li)),
) for li in xrange(num_of_layers) ])


net_out, finstate = rnn.dynamic_rnn(cells, input, initial_state=state, time_major=True)
a, spikes = net_out

log_ll = tf.reduce_mean(input.target * tf.log(a*dt) + (1.0 - input.target) * tf.log(1.0 - a*dt))

## RUNNING

state_v = tuple([ GLMStateTuple(
    np.zeros((batch_size, net_size)),
    np.zeros((batch_size, input_size)),
    np.zeros((input_size, net_size))
) for li in xrange(num_of_layers) ])

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

epochs = 500
for e in xrange(epochs):
    sess_out = sess.run(
        [
            net_out,
            finstate,
            log_ll
        ], 
        {
            input: GLMInputTuple(inputs_v, targets_v),
            state: state_v,
        }
    )

    net_out_v, finstate_v, log_ll_v = sess_out

    # output_smooth = smooth_matrix(np.squeeze(net_out_v))
    # loss = np.mean(np.square(output_smooth - target_smooth))

    for cell_id, cell in enumerate(cells._cells):
        dW = finstate_v[cell_id].dW
        assert not np.any(np.isnan(dW)), "Found nan"

        W = sess.run(cell.W)
        assign_op = cell.W.assign(W + dW)
        sess.run(assign_op)
        new_W = sess.run(cell.W)
        
    print "Epoch {}, log ll {}".format(e, log_ll_v)