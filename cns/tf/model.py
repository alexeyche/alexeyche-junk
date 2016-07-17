#!/usr/bin/env python

import os
import tensorflow as tf
import numpy as np
from tensorflow.python.ops.rnn_cell import RNNCell
from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import init_ops
from tensorflow.python.ops.math_ops import tanh
from tensorflow.python.ops.math_ops import sigmoid

def gauss_act(x, sigma):
    return tf.exp( - tf.square(-1.0 - tf.cos(x) )/( 2.0 * sigma ** 2))

def laplace_act(x, sigma):
    return tf.exp( - tf.abs(tf.cos(x) + 1.0)/( 2.0 * sigma ** 2))

def epsp_act(x, sigma):
    return tf.exp( - (tf.cos(x) + 1.0)/( 2.0 * sigma ** 2))

def simple_act(x, sigma):
    return -tf.cos(x)/2.0 + 0.5


class ThetaRNNCell(RNNCell):
    """Theta neuron RNN cell."""

    def __init__(
        self,
        num_units,
        dt,
        activation = simple_act,
        input_weights_init = tf.uniform_unit_scaling_initializer(factor=1.0),
        recc_weights_init = tf.uniform_unit_scaling_initializer(factor=1.0),
        sigma = None,
    ):
        self._num_units = num_units
        self._activation = activation
        self._dt = dt
        self._sigma = sigma if sigma else 1.0
        
        self.W = None
        self.U = None
        self.bias = None
        self.input_weights_init = input_weights_init
        self.recc_weights_init = recc_weights_init
        
        self.states_info = []

    @property
    def state_size(self):
        return self._num_units

    @property
    def output_size(self):
        return self._num_units

    def __call__(self, inputs, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            batch_size = inputs.get_shape().with_rank(2)[0]
            input_size = inputs.get_shape().with_rank(2)[1]

            self.W = vs.get_variable("W", [input_size, self._num_units], initializer=self.input_weights_init)
            self.U = vs.get_variable("U", [self._num_units, self._num_units], initializer=self.recc_weights_init)
            self.bias = vs.get_variable("Bias", [self._num_units], initializer=init_ops.constant_initializer(0.0))

            state_cos = tf.cos(state)
            weighted_input =  math_ops.matmul(inputs, self.W) + math_ops.matmul(state, self.U) + self.bias

            new_state = 1.0 - state_cos + (1.0 + state_cos) * weighted_input
            state = state + self._dt * new_state
            
            output = self._activation(new_state, self._sigma)
            
            self.states_info.append(new_state)
        return output, state


def gen_poisson(rates, T, dt, signal_form=None, seed=None):
    if seed:
        np.random.seed(seed)
    res = dt * rates >  np.random.random((T, rates.shape[0]))
    res = res.astype(np.float32)
    if signal_form is not None:
        for ni in xrange(len(rates)):
            res[:, ni] = np.convolve(signal_form, res[:, ni], mode="same")
            res[:, ni] = np.clip(res[:, ni], 0.0, 1.0)
    return res

