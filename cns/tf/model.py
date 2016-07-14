#!/usr/bin/env python

import os
import tensorflow as tf
import numpy as np
from tensorflow.python.ops.rnn_cell import RNNCell
from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import init_ops


def gauss_act(x, sigma):
    return tf.exp( - tf.square(-1.0 - tf.cos(x) )/( 2.0 * sigma ** 2))

def laplace_act(x, sigma):
    return tf.exp( - tf.abs(tf.cos(x) + 1.0)/sigma)

def epsp_act(x, sigma):
    return tf.exp( - (tf.cos(x) + 1.0)/sigma)


SEED=3
INIT_FACTOR = 10.0

class ThetaRNNCell(RNNCell):
    """Theta neuron RNN cell."""

    def __init__(
        self,
        num_units,
        dt,
        sigma,
        input_weights_init=init_ops.constant_initializer(INIT_FACTOR), #tf.uniform_unit_scaling_initializer(factor=INIT_FACTOR, seed=SEED),
        recc_weights_init=init_ops.constant_initializer(INIT_FACTOR),
        activation=laplace_act
    ):
        self._num_units = num_units
        self._activation = activation
        self._dt = dt
        self._sigma = sigma

        self.W = None
        self.U = None
        self.bias = None
        self.input_weights_init = input_weights_init
        self.recc_weights_init = recc_weights_init
        self.states_info = []
        self.weighted_input_info = []

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
            #self.U = vs.get_variable("U", [self._num_units, self._num_units], initializer=self.recc_weights_init)
            self.bias = vs.get_variable("Bias", [self._num_units], initializer=init_ops.constant_initializer(0.0))

            state_cos = tf.cos(state)
            #weighted_input =  math_ops.matmul(inputs, tf.exp(self.W)) + math_ops.matmul(state_cos, tf.exp(self.U)) + self.bias
            weighted_input =  math_ops.matmul(inputs, self.W) + self.bias # + math_ops.matmul(state_cos, self.U)

            new_state = state + self._dt * (1.0 - state_cos + (1.0 + state_cos) * weighted_input)
            output = self._activation(new_state, self._sigma)
            # output = -tf.cos(new_state)/2.0 + 0.5
            self.states_info.append(new_state)
            self.weighted_input_info.append(weighted_input)
        return output, new_state

    def get_signal_form(self, len=10, sigma_bias = 0.2, scope=None):
        with vs.variable_scope(scope or type(self).__name__ + "_SignalForm"):
			sess = tf.Session()
			state = tf.placeholder(tf.float32, shape=(len,), name="State")
			act = self._activation(state, sigma_bias * self._sigma)
			
			sess.run(tf.initialize_all_variables())
			act_v = sess.run(act, {state: np.linspace(3.14/2.0, 3.14*3.0/2.0, len)})
			return act_v
			   

def gen_poisson(rates, T, dt, seed):
    np.random.seed(seed)
    res = dt * rates >  np.random.random((T, rates.shape[0]))
    return res.astype(np.float32)


def generate_data(input_size, net_size, seq_size, batch_size, signal_form):
    data = [ np.zeros((batch_size, input_size)) for _ in xrange(seq_size) ]
    assert input_size == seq_size

    for seq_i in xrange(seq_size):
       data[seq_i][0, seq_i] = 1.0
       #data[seq_i][0, :] = np.convolve(signal_form, data[seq_i][0, :], mode="same")


    #target_seq = gen_poisson(np.asarray([4.0]*net_size), seq_size, 0.01)

    target_seq = np.zeros((seq_size, net_size))
    target_seq[seq_size/2, 0] = 1.0

    for ni in xrange(net_size):
      target_seq[:, ni] = np.convolve(signal_form, target_seq[:, ni], mode="same")


    target_v = [ np.asarray([ target_seq[si, :] for _ in xrange(batch_size) ]) for si in xrange(seq_size) ]

    return data, target_v
