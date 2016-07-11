#!/usr/bin/env python

import os
import tensorflow as tf
import numpy as np
from tensorflow.python.ops import control_flow_ops
from tensorflow.python.ops.rnn_cell import RNNCell
from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import rnn
from tensorflow.python.ops import init_ops

from matplotlib import pyplot as plt

sigma = 0.001


def gauss_act(x, sigma=sigma):
    return tf.exp( - tf.square(-1.0 - tf.cos(x) )/( 2.0 * sigma ** 2)) 

def gauss_act_basic(x, m, sigma=sigma):
    return np.exp( - np.square(m - x)/( 2.0 * sigma ** 2)) 

class ThetaRNNCell(RNNCell):
    """Theta neuron RNN cell."""

    def __init__(
        self, 
        num_units, 
        dt = 0.01, 
        input_weights_init=tf.uniform_unit_scaling_initializer(), 
        recc_weights_init=tf.uniform_unit_scaling_initializer(), 
        activation=gauss_act
    ):
        self._num_units = num_units
        self._activation = activation
        self._dt = dt
        
        self.W = None
        self.U = None
        self.bias = None
        self.input_weights_init = input_weights_init
        self.recc_weights_init = recc_weights_init
        
    @property
    def state_size(self):
        return self._num_units

    @property
    def output_size(self):
        return self._num_units

    def __call__(self, inputs, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            input_size = inputs.get_shape().with_rank(2)[1]
            
            self.W = vs.get_variable("W", [input_size, self._num_units], initializer=self.input_weights_init)
            self.U = vs.get_variable("U", [self._num_units, self._num_units], initializer=self.recc_weights_init)
            self.bias = vs.get_variable("Bias", [self._num_units], initializer=init_ops.constant_initializer(0.0))

            state_cos = tf.cos(state)
            weighted_input = math_ops.matmul(inputs, self.W) + math_ops.matmul(state_cos, self.U) + self.bias
            new_state = state + self._dt * (1.0 - state_cos + (1.0 + state_cos) * weighted_input)
            
            output = self._activation(new_state)
        return output, new_state



def gen_poisson(rates, T, dt):
    res = dt * rates >  np.random.random((T, rates.shape[0]))
    return res.astype(np.float32)


def gen_form():
    thetaNeuron = ThetaRNNCell(1, input_weights_init = tf.constant_initializer(1))

    inputs  = 25 * [tf.placeholder(tf.float32, shape=(1, 1), name="Igenform")]
    init_state = tf.zeros((1, 1))

    outputs, state = rnn.rnn(thetaNeuron, inputs, init_state, scope="GenForm")

    init = tf.initialize_all_variables()
    sess = tf.Session()
    data = 25 * [np.asarray([[0.0]])]
    data[13] = np.asarray([[10000.0]])

    with tf.device("/cpu:0"):
        sess.run(init)
        feed_dict = {k: v for k, v in zip(inputs, data)}
        eo, _ = sess.run([outputs, state], feed_dict)
    return np.asarray(eo)[:,0,0]



# M = 5
# batch_size = 1
# N = 5
# dt = 0.01

# seq_size = 200

# thetaNeuron = ThetaRNNCell(N, dt)

# inputs  = seq_size * [tf.placeholder(tf.float32, shape=(batch_size, M), name="I")]
# init_state = tf.placeholder(tf.float32, shape=(batch_size, N), name="State")

# outputs, state = rnn.rnn(thetaNeuron, inputs, init_state)

# init = tf.initialize_all_variables()

# sess = tf.Session()


# data = [ 100.0*np.random.randn(batch_size, M) for _ in xrange(seq_size) ]


# target_data = gen_poisson(np.asarray([10]*N), seq_size, 0.01)


# with tf.device("/cpu:0"):
#     sess.run(init)
#     feed_dict = {k: v for k, v in zip(inputs, data)}
#     feed_dict[init_state] = np.zeros((batch_size, N))
#     eo, es = sess.run([outputs, state], feed_dict)


# eo = np.asarray(eo)
# out_one = eo[:,0,0]
# out = eo[:,0, :]

# plt.figure(1)
# plt.subplot(2,1,1)
# plt.plot(out_one)
# plt.subplot(2,1,2)
# plt.imshow(out.T)
# plt.show()

form = gen_form()
print form