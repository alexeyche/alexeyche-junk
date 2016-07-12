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

sigma = 0.0005
dt = 0.01

def gauss_act(x, sigma=sigma):
    return tf.exp( - tf.square(-1.0 - tf.cos(x) )/( 2.0 * sigma ** 2))

def gauss_act_basic(x, m, sigma=sigma):
    return np.exp( - np.square(m - x)/( 2.0 * sigma ** 2))

class ThetaRNNCell(RNNCell):
    """Theta neuron RNN cell."""

    def __init__(
        self,
        num_units,
        dt = dt,
        input_weights_init=tf.random_uniform_initializer(minval=2.0, maxval=3.0),
        recc_weights_init=tf.random_uniform_initializer(minval=2.0, maxval=3.0), 
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
        self.debug_data = []

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
            #weighted_input =  math_ops.matmul(inputs, tf.exp(self.W)) + math_ops.matmul(state_cos, tf.exp(self.U)) + self.bias
            weighted_input =  math_ops.matmul(inputs, self.W) + math_ops.matmul(state_cos, self.U) + self.bias

            new_state = state + self._dt * (1.0 - state_cos + (1.0 + state_cos) * weighted_input)
            output = self._activation(new_state)
            self.debug_data.append((new_state, weighted_input))
        return output, new_state



def gen_poisson(rates, T, dt):
    res = dt * rates >  np.random.random((T, rates.shape[0]))
    return res.astype(np.float32)


def get_form():
    #thetaNeuron = ThetaRNNCell(1, input_weights_init = tf.constant_initializer(np.log(1.0/dt)), recc_weights_init = tf.constant_initializer(0))
    thetaNeuron = ThetaRNNCell(1, input_weights_init = tf.constant_initializer(1.0/dt), recc_weights_init = tf.constant_initializer(0))

    inputs  = [ tf.placeholder(tf.float32, shape=(1, 1), name="Input_{}".format(idx)) for idx in xrange(12) ]
    init_state = tf.zeros((1, 1))

    outputs, state = rnn.rnn(thetaNeuron, inputs, init_state, scope="GenForm")

    init = tf.initialize_all_variables()
    sess = tf.Session()
    data = [ np.asarray([[3.0/2.0]]) ] + [ np.zeros((1, 1)) for _ in xrange(len(inputs)-1) ]

    with tf.device("/cpu:0"):
        sess.run(init)
        feed_dict = {k: v for k, v in zip(inputs, data)}
        outputs_v, _ = sess.run([outputs, state], feed_dict)
    signal_form = np.asarray(outputs_v)[:,0,0]
    return signal_form[np.where(signal_form > 1e-08)]


signal_form = get_form()

input_size = 10
batch_size = 1
net_size = 10
epochs = 100
seq_size = 200

thetaNeuron = ThetaRNNCell(net_size, dt)
inputs  = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input_{}".format(idx)) for idx in xrange(seq_size) ]

init_state = tf.placeholder(tf.float32, shape=(batch_size, net_size), name="State")

outputs, state = rnn.rnn(thetaNeuron, inputs, init_state)


sess = tf.Session()


data = [ np.zeros((batch_size, input_size)) for _ in xrange(seq_size) ]

for seq_i in xrange(seq_size):
    ni = seq_i % input_size
    data[seq_i][0, ni] = 1.0 
    data[seq_i][0, :] = np.convolve(signal_form, data[seq_i][0, :], mode="same")


target_seq = gen_poisson(np.asarray([4.0]*net_size), seq_size, 0.01)

for ni in xrange(net_size):
    target_seq[:, ni] = np.convolve(signal_form, target_seq[:, ni], mode="same")    

target_data = [ np.asarray([ target_seq[si, :] for _ in xrange(batch_size) ]) for si in xrange(seq_size) ]


targets = [ tf.placeholder(tf.float32, shape=(batch_size, net_size), name="Target") for si in xrange(seq_size) ]
loss = tf.add_n([ tf.nn.l2_loss(target - outputs) for output, target in zip(outputs, targets) ])
train_step = tf.train.AdamOptimizer(0.5).minimize(loss)

init = tf.initialize_all_variables()

weights = []
with tf.device("/gpu:0"):
    sess.run(init)
    for e in xrange(epochs):
        feed_dict = {k: v for k, v in zip(inputs, data)}
        feed_dict[init_state] = np.zeros((batch_size, net_size))
        feed_dict.update({k: v for k, v in zip(targets, target_data)})

        fetch = [outputs, state]
        fetch += [thetaNeuron.W]
        fetch += [loss, train_step]
        
        out = sess.run(fetch, feed_dict)
        outputs_v, state_v = np.asarray(out[0]), out[1]
        W = out[2]
        loss_v, train_step_v = out[3], out[4]
        print "Epoch {}, train loss {}".format(e, loss_v)
        weights.append(W)


out_one = outputs_v[:,0,0]
out = outputs_v[:,0, :]

plt.figure(1)
plt.subplot(2,1,1)
plt.imshow(target_seq)
plt.subplot(2,1,2)
plt.imshow(out.T)
plt.show()

