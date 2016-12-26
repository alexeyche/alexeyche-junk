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

from util import sm, sl, smooth_matrix, smooth, moving_average, norm, outer

np.random.seed(10)
tf.set_random_seed(10)


seq_size = 1000

weight_init_factor = 1.0
dt_sec = 1.0/1000.0
dt = dt_sec * 1000.0

dt = 0.25

lrate = 1e-02

batch_size = 1
filters_num = 90
L = 100
strides = 1
net_size = 100

tau_syn = dt/10.0
tau_mem = dt/10.0
tau_refr = dt/2.0
tau_learn = dt/10.0
tau_long = dt/float(seq_size)
amp_refr = 100.0

a = 0.08
b = 1.0
c = 0.8

env = Env("simple_test")
_FHNStateTuple = collections.namedtuple("FHNStateTuple", ("V", "W"))
_FHNOutputTuple = collections.namedtuple("FHNOutputTuple", ("V", "W"))

class FHNStateTuple(_FHNStateTuple):
    __slots__ = ()

class FHNOutputTuple(_FHNOutputTuple):
    __slots__ = ()


def basic_v_relation(V):
    return V - V * V * V/3.0


class FHNCell(rc.RNNCell):
    def __init__(self, num_units, f):
        self._num_units = num_units
        self._f = f
        
        self._a = None
        self._b = None
        self._c = None
        self.Wr = None
        self.W = None

        self._state_is_tuple = True

    @property
    def state_size(self):
        return FHNStateTuple(
            self._num_units,
            self._num_units,
        )

    @property
    def output_size(self):
        return FHNOutputTuple(
            self._num_units, 
            self._num_units
        )

    def _init_parameters(self, input):
        if self._a is None:
            self._a = vs.get_variable("a", [self._num_units], initializer=tf.constant_initializer(value=a))
        if self._b is None:
            self._b = vs.get_variable("b", [self._num_units], initializer=tf.constant_initializer(value=b))
        if self._c is None:
            self._c = vs.get_variable("c", [self._num_units], initializer=tf.constant_initializer(value=c))
        if self.W is None:
            dim = input.get_shape()[1]
            self.W = vs.get_variable("W", [dim, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.Wr is None:
            self.Wr = vs.get_variable("Wr", [self._num_units, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))

    def __call__(self, input, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            self._init_parameters(input)
            
            V, W = state
            
            input_transf = tf.matmul(input, self.W) 

            dV = self._f(V) - W + input_transf + tf.matmul(V, self.Wr) + tf.random_normal((self._num_units,)) * 1.0
            dW = self._a * (self._b * V - W)

            # dV2 = self._f(V + 0.6666 * dV * dt) - W + input_transf + tf.matmul(V + 0.6666 * dV * dt, self.Wr)     
            # dW2 = self._a * (self._b * V - (W + 0.6666 * dW * dt))            
            
            # V += dt * (0.25 * dV + 0.75 * dV2)
            # W += dt * (0.25 * dW + 0.75 * dW2)

            V += dt * dV
            W += dt * dW

            return FHNOutputTuple(V, W), FHNStateTuple(V, W)


input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, 1), name="Input")
target = tf.placeholder(tf.float32, shape=(seq_size, batch_size, net_size), name="Target")

state = FHNStateTuple(
    tf.placeholder(tf.float32, [batch_size, net_size], name="V"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="W"),
)


cell = FHNCell(net_size, basic_v_relation) 

net_out, finstate = rnn.dynamic_rnn(cell, input, initial_state=state, time_major=True)

V = tf.expand_dims(net_out.V, 3)
V = tf.transpose(V, [1, 0, 3, 2])


recov_filter = vs.get_variable("R", [L, 1, net_size, 1], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))            
output = tf.nn.conv2d(V, recov_filter, strides=[1, strides, 1, 1], padding='SAME')
output = tf.transpose(output, [1, 0, 2, 3])
output = tf.squeeze(output, 3)

input_n = tf.nn.l2_normalize(input, 0)
output_n = tf.nn.l2_normalize(output, 0)

loss = tf.nn.l2_loss(output_n - input_n)

# train_step = tf.train.AdamOptimizer(0.00001).minimize(loss)
# train_step = tf.train.GradientDescentOptimizer(0.0001).minimize(loss)
train_step = tf.train.FtrlOptimizer(0.001).minimize(loss) # not bad
# train_step = tf.train.RMSPropOptimizer(0.000001).minimize(loss)
# train_step = tf.train.ProximalAdagradOptimizer(0.000001).minimize(loss)

sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("fhn_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())

tmp_dir = pj(os.environ["HOME"], "tmp")
[ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]


targets_v = np.zeros((seq_size, net_size))

for si in xrange(seq_size):
    if si % net_size == 0:
        targets_v[si, int((float(si)/seq_size) * net_size)] = 1.0

targets_v = smooth_matrix(targets_v)
inputs_v =  moving_average(np.random.randn(seq_size), 10).reshape(seq_size, batch_size, 1)


epochs = 1000
for e in xrange(epochs):
    state_v = FHNStateTuple(
        np.zeros((batch_size, net_size)),
        np.zeros((batch_size, net_size)),
    )


    sess_out = sess.run(
        [
            net_out,
            finstate,
            train_step,
            loss,
            input_n,
            output_n
        ],
        {
            input: inputs_v,
            state: state_v,
            target: targets_v.reshape(seq_size, batch_size, net_size)
        }
    )

    net_out_v, finstate_v, _, loss_v, input_n_v, output_n_v = sess_out

    Vv, Wv = net_out_v

    sm(Vv, file=pj(tmp_dir, "V_{}.png".format(e)))
    sm(Wv, file=pj(tmp_dir, "W_{}.png".format(e)))
    sl(output_n_v, input_n_v, (output_n_v-input_n_v) ** 2, file=pj(tmp_dir, "rec_{}.png".format(e)))

    print "Epoch {}, loss {}".format(e, loss_v)
























