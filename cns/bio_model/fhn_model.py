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
            self._a = vs.get_variable("a", [self._num_units], initializer=tf.constant_initializer(value=a), trainable=False)
        if self._b is None:
            self._b = vs.get_variable("b", [self._num_units], initializer=tf.constant_initializer(value=b), trainable=False)
        if self._c is None:
            self._c = vs.get_variable("c", [self._num_units], initializer=tf.constant_initializer(value=c), trainable=False)
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

            dV = self._f(V) - W + input_transf + tf.matmul(V, self.Wr) # + tf.random_normal((self._num_units,)) * 0.1
            dW = self._a * (self._b * V - self._c * W)

            # dV2 = self._f(V + 0.6666 * dV * dt) - W + input_transf + tf.matmul(V + 0.6666 * dV * dt, self.Wr)
            # dW2 = self._a * (self._b * V - (W + 0.6666 * dW * dt))

            # V += dt * (0.25 * dV + 0.75 * dV2)
            # W += dt * (0.25 * dW + 0.75 * dW2)

            V += dt * dV
            W += dt * dW

            return FHNOutputTuple(V, W), FHNStateTuple(V, W)


state = FHNStateTuple(
    tf.placeholder(tf.float32, [batch_size, net_size], name="V"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="W"),
)


cell = FHNCell(net_size, basic_v_relation)

input = tf.placeholder(tf.float32, shape=(batch_size, seq_size, 1, 1), name="Input")
filter = vs.get_variable("E", [L, 1, 1, filters_num], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))

conv_out = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='SAME')
conv_out = tf.transpose(conv_out, [1,0,2,3])
conv_out = tf.squeeze(conv_out, squeeze_dims=[2])

net_out, finstate = rnn.dynamic_rnn(cell, conv_out, initial_state=state, time_major=True)

V = tf.expand_dims(net_out.V, 3)
V = tf.transpose(V, [1, 0, 3, 2])


recov_filter = vs.get_variable("R", [L, 1, net_size, 1], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
output = tf.nn.conv2d(V, recov_filter, strides=[1, strides, 1, 1], padding='SAME')

# output = tf.transpose(output, [1, 0, 2, 3])
# output = tf.squeeze(output, squeeze_dims=[2])

input_n = tf.nn.l2_normalize(input, 1)
output_n = tf.nn.l2_normalize(output, 1)

loss = tf.nn.l2_loss(output_n - input_n)

# optimizer = tf.train.AdamOptimizer(0.001)
# optimizer = tf.train.GradientDescentOptimizer(0.01)
# optimizer = tf.train.MomentumOptimizer(0.0001, 0.9, use_nesterov=True)
# optimizer = tf.train.FtrlOptimizer(0.00001) # not bad
optimizer = tf.train.RMSPropOptimizer(0.000001)
# optimizer = tf.train.AdagradOptimizer(0.0001)
# optimizer = tf.train.AdadeltaOptimizer(0.0001)
# optimizer = tf.train.ProximalAdagradOptimizer(0.0001)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(loss, tvars)
# grads = grads_raw

grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

apply_grads = optimizer.apply_gradients(zip(grads, tvars))


sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("fhn_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())

tmp_dir = pj(os.environ["HOME"], "tmp", "tf_pics")
[ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]


inputs_v =  moving_average(np.random.randn(seq_size), 10).reshape(batch_size, seq_size, 1, 1)


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
            apply_grads,
            loss,
            input_n,
            output_n,
            grads,
            grads_raw
        ],
        {
            input: inputs_v,
            state: state_v,
        }
    )

    net_out_v, finstate_v, _, loss_v, input_n_v, output_n_v, grads_v, grads_raw_v = sess_out

    for g, gr, tv in zip(grads_v, grads_raw_v, tvars):
        vname = tv.name.split("/")[-1].replace(":","_")

        f = pj(tmp_dir, "{}_{}.png".format(vname, e))
        fr = pj(tmp_dir, "{}_raw_{}.png".format(vname, e))

        if len(g.shape) == 1:
            sl(g, file=f)
            sl(gr, file=fr)
        else:
            sm(g, file=f)
            sm(gr, file=fr)

    Vv, Wv = net_out_v
    sm(Vv, file=pj(tmp_dir, "out_V_{}.png".format(e)))
    sm(Wv, file=pj(tmp_dir, "state_W_{}.png".format(e)))
    sl(output_n_v, input_n_v, (output_n_v-input_n_v) ** 2, file=pj(tmp_dir, "rec_{}.png".format(e)))

    print "Epoch {}, loss {}".format(e, loss_v)
























