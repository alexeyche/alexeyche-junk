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
from tensorflow.python.framework import dtypes

from hist_rnn import cat_hist, rnn_with_hist_loop_fn

np.random.seed(10)
tf.set_random_seed(10)

def sm(matrix):
    plt.imshow(np.squeeze(matrix).T)
    plt.colorbar()
    plt.show()

def sl(*vector, **kwargs):
    for id, v in enumerate(vector):
        plt.plot(np.squeeze(v))
    
    if kwargs.get("file"):
        plt.savefig(kwargs["file"])
        plt.clf()
    else:
        plt.show()

def moving_average(a, n=3) :
    ret = np.cumsum(a, dtype=float)
    ret[n:] = ret[n:] - ret[:-n]
    v = ret[n - 1:] / n
    return np.pad(v, [0, n-1], 'constant')

def norm(data):
    data_denom = np.sqrt(np.sum(data ** 2))
    data = data/data_denom
    return data


weight_init_factor = 1.0
L = 100
filters_num = 90
batch_size = 1
max_time = 1000
num_units = 50
input_depth = 1

_OutputTuple = collections.namedtuple("OutputTuple", ("output", "output_inner", "input", "output_history"))
_StateTuple = collections.namedtuple("StateTuple", ("output_history", "state"))


class OutputTuple(_OutputTuple):
    __slots__ = ()

class StateTuple(_StateTuple):
    __slots__ = ()



class BasicRNNCellWitHist(tf.nn.rnn_cell.RNNCell):
    def __init__(self, num_units, activation=tf.nn.tanh):
        self._num_units = num_units
        self._activation = activation
        self.W = None
        self.Wr = None
        self.F = None
        self.R = None

    @property
    def state_size(self):
        return StateTuple(
            self._num_units, # it means actually nothing if we provide starting state
            self._num_units
        )

    @property
    def output_size(self):
        return OutputTuple(1, self._num_units, L, L) 

    def _init_parameters(self):
        if self.F is None:
            self.F = vs.get_variable("F", [L, filters_num], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.W is None:
            self.W = vs.get_variable("W", [filters_num , self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.Wr is None:
            self.Wr = vs.get_variable("Wr", [self._num_units , self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.R is None:
            self.R = vs.get_variable("R", [L, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))


    def __call__(self, input_tuple, state_tuple, scope=None):
        inputs, targets = input_tuple
        assert inputs.get_shape()[2] == 1, "Need input depth == 1" 
        inputs = tf.squeeze(inputs, 2)
        print inputs.get_shape()
        with vs.variable_scope(scope or "basic_rnn_cell"):
            self._init_parameters()
            
            output_history, state = state_tuple    

            inputs_filtered = tf.matmul(inputs, self.F)

            output = self._activation(tf.matmul(inputs_filtered, self.W) + tf.matmul(state, self.Wr))

            output_history = cat_hist(output_history, output, 1)

            output_history_reshaped = tf.reshape(tf.transpose(output_history, [0, 2, 1]), [batch_size * self._num_units, L])
            
            output_point = tf.matmul(output_history_reshaped, self.R)
            output_point = tf.reshape(output_point, [batch_size, self._num_units, self._num_units])
            output_point = tf.reduce_sum(output_point, [1, 2])
            output_point = tf.expand_dims(output_point, 1)

        return OutputTuple(output_point, output, inputs, tf.reshape(output_history[:, :, 0], [batch_size, L])), StateTuple(output_history, output)




sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)

inputs = tf.placeholder(tf.float32, shape=(max_time, batch_size, input_depth), name="Input")
targets = tf.placeholder(tf.float32, shape=(max_time, batch_size, input_depth), name="Targets")

cell = BasicRNNCellWitHist(num_units)
state = StateTuple(
    tf.placeholder(tf.float32, [batch_size, L, num_units], name="output_history"),
    tf.placeholder(tf.float32, [batch_size, num_units], name="output"),
)

output_tuple, final_state, _ = tf.nn.raw_rnn(cell, rnn_with_hist_loop_fn(inputs, targets, sequence_length, state, L))
outputs_ta, outputs_inner_ta, inputs_ta, outputs_hist_ta = output_tuple

outputs = outputs_ta.stack()
outputs_inner = outputs_inner_ta.stack()

outputs_n = tf.nn.l2_normalize(outputs, 0)
inputs_n = tf.nn.l2_normalize(inputs, 0)
loss = tf.nn.l2_loss(outputs_n - inputs_n)

train_step = tf.train.AdadeltaOptimizer(0.05).minimize(loss)

input_history = inputs_ta.stack()
output_history = outputs_hist_ta.stack()

sess = tf.Session()
sess.run(tf.global_variables_initializer())
inputs_v =  moving_average(np.random.randn(max_time), 10).reshape(max_time, batch_size, input_depth)

epochs = 1
for e in xrange(epochs):
    state_v = StateTuple(
        np.zeros((batch_size, L, num_units)),
        np.zeros((batch_size, num_units)),
    )

    outputs_v, inputs_n_v, outputs_inner_v, input_history_v, output_history_v, loss_v, final_state_v, gr_v, _ = sess.run(
        [
            outputs_n,
            inputs_n, 
            outputs_inner,
            input_history, 
            output_history, 
            loss, 
            final_state, 
            tf.gradients([loss], [cell.R])[0],
            train_step
        ], 
        {
            inputs: inputs_v, 
            sequence_length: np.asarray([max_time]*batch_size),
            state: state_v,
        }
    )
    sl(outputs_v, inputs_n_v, (outputs_v-inputs_n_v) ** 2, file=pj(os.environ["HOME"], "tmp", "rnn_conv_{}.png".format(e)))

    print "Epoch {}, loss {}".format(e, loss_v)
