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

seq_size = 1000

factor = 0.1
dt = 1.0/1000.0
N = 1
lrate = 1e-03
epochs = 1000

net_size = 10
num_of_layers = 1
batch_size = 1
seq_size = 1000
input_size = 100

sigma = 1.0/seq_size 
env = Env("simple_test")

_GLMStateTuple = collections.namedtuple("GLMStateTuple", ("u", "s"))


class GLMStateTuple(_GLMStateTuple):
  __slots__ = ()


class GLMCell(rc.RNNCell):
    def __init__(self, num_units, activation = mo.sigmoid):
        self._num_units = num_units
        self._activation = activation

        self.W = None
        self._state_is_tuple = True

    @property
    def state_size(self):
	    return GLMStateTuple(self._num_units, input_size)

    @property
    def output_size(self):
        return self._num_units


    def _init_parameters(self):
        if self.W is None:
        	self.W = vs.get_variable("W", [input_size, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))

    def __call__(self, inputs, state, scope=None):
	    with vs.variable_scope(scope or type(self).__name__):
	    	self._init_parameters()

	    	u, s = state

	    	s = (1.0 - sigma) * s + sigma * inputs	        
	        u = (1.0 - sigma) * u + sigma * mo.matmul(s, self.W)

	        act = self._activation(u)

	        return act, GLMStateTuple(u, s)





cell_type = GLMCell

cells = rc.MultiRNNCell([cell_type(net_size) for _ in xrange(num_of_layers)], state_is_tuple=True)

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")

state = tuple([ GLMStateTuple(
	tf.placeholder(tf.float32, [batch_size, net_size], name="u{}".format(li)),
	tf.placeholder(tf.float32, [batch_size, input_size], name="s{}".format(li)),
) for li in xrange(num_of_layers) ])


net_out, finstate = rnn.dynamic_rnn(cells, input, initial_state=state, time_major=True)


## RUNNING

state_v = tuple([ GLMStateTuple(
	np.zeros((batch_size, net_size)),
	np.zeros((batch_size, input_size))
) for li in xrange(num_of_layers) ])


inputs_v = np.zeros((seq_size, batch_size, input_size))

x_rate = 2.0
for bi in xrange(batch_size):
	for tt in xrange(seq_size):
	    for ni in xrange(input_size):
	        if np.random.random() <= x_rate/seq_size:        
	            inputs_v[tt, bi, ni] = 1.0




sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("conv_rnn_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.initialize_all_variables())


sess_out = sess.run(
    [
        net_out,
        finstate,
    ], 
    {
        input: inputs_v,
        state: state_v,
    }
)

net_out_v, finstate_v = sess_out
