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

import math
import scipy
import scipy.sparse

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
        activation = simple_act,
        input_weights_init = tf.uniform_unit_scaling_initializer(factor=1.0),
        recc_weights_init = tf.uniform_unit_scaling_initializer(factor=1.0),
        sigma = None,
        update_gate = True,
        dt = 0.25
    ):
        self._num_units = num_units
        self._activation = activation
        self._dt = dt
        self._sigma = sigma if sigma else 1.0
        self._update_gate = update_gate

        self.W = None
        self.U = None
        self.bias = None
        self.W_u = None
        self.U_u = None
        self.bias_u = None
        self.W_s = None
        self.U_s = None
        self.bias_s = None
        
        self.input_weights_init = input_weights_init
        self.recc_weights_init = recc_weights_init
        
        self._sensitivity = False
        
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
            if self.W is None:
                self.W = vs.get_variable("W", [input_size, self._num_units], initializer=self.input_weights_init)
            if self.U is None:
                self.U = vs.get_variable("U", [self._num_units, self._num_units], initializer=self.recc_weights_init)
            if self.bias is None:
                self.bias = vs.get_variable("Bias", [self._num_units], initializer=init_ops.constant_initializer(0.0))

            if self._sensitivity:
                if self.W_s is None: 
                    self.W_s = vs.get_variable("W_s", [input_size, self._num_units], initializer=self.input_weights_init)
                if self.U_s is None:
                    self.U_s = vs.get_variable("U_s", [self._num_units, self._num_units], initializer=self.recc_weights_init)
                if self.bias_s is None:
                    self.bias_s = vs.get_variable("Bias_s", [self._num_units], initializer=init_ops.constant_initializer(0.0))
                s = sigmoid(math_ops.matmul(inputs, self.W_s) + math_ops.matmul(state, self.U_s) + self.bias_s)
                s *= 3.0
            else:
                s = 1.0

            state_cos = s * tf.cos(state)
            weighted_input =  math_ops.matmul(inputs, self.W) + math_ops.matmul(state, self.U) + self.bias

            new_state = s - state_cos + (s + state_cos) * weighted_input
            if not self._update_gate:
                state = state + self._dt * new_state
            else:
                if self.W_u is None: 
                    self.W_u = vs.get_variable("W_u", [input_size, self._num_units], initializer=self.input_weights_init)
                if self.U_u is None:
                    self.U_u = vs.get_variable("U_u", [self._num_units, self._num_units], initializer=self.recc_weights_init)
                if self.bias_u is None:
                    self.bias_u = vs.get_variable("Bias_u", [self._num_units], initializer=init_ops.constant_initializer(0.0))
                u = sigmoid(math_ops.matmul(inputs, self.W_u) + math_ops.matmul(state, self.U_u) + self.bias_u)
                state = u * state + (1.0-u) * self._dt * new_state

            output = self._activation(new_state, self._sigma)
            
            self.states_info.append(state)

        return output, state




