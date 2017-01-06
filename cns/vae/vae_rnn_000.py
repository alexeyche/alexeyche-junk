
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

from util import sm, sl, smooth_matrix, smooth, moving_average, norm, outer, generate_dct_dictionary

def xavier_init(fan_in, fan_out, constant=1):
    low = -constant*np.sqrt(6.0/(fan_in + fan_out))
    high = constant*np.sqrt(6.0/(fan_in + fan_out))
    return tf.random_uniform((fan_in, fan_out),
                             minval=low, maxval=high,
                             dtype=tf.float32)

np.random.seed(10)
tf.set_random_seed(10)

seq_size = 1000

batch_size = 1
filters_num = 90
L = 100
strides = 1
net_size = 100

h0_size = 125
h1_size = 125
latent_dim = 2
lam = 0
weight_init_factor = 1.0

_VAEOutputTuple = collections.namedtuple("VAEOutputTuple", ("mean", "var"))

class VAEOutputTuple(_VAEOutputTuple):
    __slots__ = ()


class VAECell(rc.RNNCell):
    def __init__(self, base_cell):
        self._base_cell = base_cell
        self.Bmean = None
        self.Bvar = None
        self.Wmean = None
        self.Wvar = None
        self.Wmean_r = None
        self.Wvar_r = None
        self._num_units = self._base_cell.state_size

    @property
    def state_size(self):
        return self._base_cell.state_size

    @property
    def output_size(self):
        return VAEOutputTuple(self._base_cell.output_size, self._base_cell.output_size)

    def _init_parameters(self):
        if self.Bmean is None:
            self.Bmean = vs.get_variable("Bmean", [self._num_units], initializer = tf.zeros_initializer())
        if self.Bvar is None:
            self.Bvar = vs.get_variable("Bvar", [self._num_units], initializer = tf.zeros_initializer())
        if self.Wmean is None:
            self.Wmean = vs.get_variable("Wmean", [self._num_units, self._num_units], initializer = tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.Wvar is None:
            self.Wvar = vs.get_variable("Wvar", [self._num_units, self._num_units], initializer = tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.Wmean_r is None:
            self.Wmean_r = vs.get_variable("Wmean_r", [self._num_units, self._num_units], initializer = tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.Wvar_r is None:
            self.Wvar_r = vs.get_variable("Wvar_r", [self._num_units, self._num_units], initializer = tf.uniform_unit_scaling_initializer(factor=weight_init_factor))


    def __call__(self, input, state, scope=None):
        self._init_parameters()

        output, state = self._base_cell(input, state, scope)
        
        mean_e = tf.matmul(output, self.Wmean) + tf.matmul(state, self.Wmean_r) + self.Bmean
        var_e = tf.matmul(output, self.Wvar) + tf.matmul(state, self.Wvar_r) + self.Bvar
        
        return VAEOutputTuple(mean_e, var_e), state


env = Env("vae_run")

cell = VAECell(tf.nn.rnn_cell.BasicRNNCell(net_size))

input = tf.placeholder(tf.float32, shape=(batch_size, seq_size, 1, 1), name="Input")

init = lambda shape, dtype: generate_dct_dictionary(L, h0_size).reshape(L, 1, 1, h0_size)

filter = tf.Variable(generate_dct_dictionary(L, h0_size).reshape(L, 1, 1, h0_size), name="F", dtype=tf.float32)

input_filtered = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='SAME')
input_filtered = tf.squeeze(input_filtered, 0)
# input_filtered = tf.transpose(input_filtered, [1, 0, 2])

state = tf.placeholder(tf.float32, [batch_size, net_size], name="state")

net_out, finstate = rnn.dynamic_rnn(cell, input_filtered, initial_state=state, time_major=True)

epsilon = tf.random_normal(tf.shape(net_out.var), name='epsilon')
std_encoder = tf.exp(0.5 * net_out.var)

z = net_out.mean + tf.mul(std_encoder, epsilon)

z = tf.transpose(z, [1, 0, 2])
z = tf.expand_dims(z, 2)

filter_r = tf.Variable(generate_dct_dictionary(L, net_size).reshape(L, 1, net_size, 1), name="R", dtype=tf.float32)
input_hat = tf.nn.conv2d(z, filter_r, strides=[1, strides, 1, 1], padding='SAME')

KLD = -0.5 * tf.reduce_sum(1 + net_out.var - tf.pow(net_out.mean, 2) - tf.exp(net_out.var), reduction_indices=[1, 2])

BCE = tf.square(tf.reduce_sum(input_hat - input, reduction_indices=[0, 2, 3]))/2.0

loss = tf.reduce_mean(BCE + KLD)

train_step = tf.train.AdamOptimizer(0.001).minimize(loss)

# ###############################################################



sess = tf.Session()

model_fname = env.run("model.ckpt")
saver = tf.train.Saver()
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0 
else:
    sess.run(tf.global_variables_initializer())


tmp_dir = pj(os.environ["HOME"], "tmp", "tf_pics")
[ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]


input_v =  moving_average(np.random.randn(seq_size), 50).reshape(1, seq_size, batch_size, 1)

epochs = 1000
for e in xrange(epochs):
    input_filtered_v, net_out_v, z_v, input_hat_v, KLD_v, BCE_v, loss_v, _ = sess.run([
        input_filtered, net_out, z, input_hat, KLD, BCE, loss, train_step
    ], {
        input: input_v,
        state: np.zeros((batch_size, net_size))
    })

    sl(input_v, input_hat_v, (input_v-input_hat_v) ** 2, file=pj(tmp_dir, "rec_{}.png".format(e)))
    sm(z_v, file=pj(tmp_dir, "latent_{}.png".format(e)))

    print "Epoch {}, loss {}, KLD {}".format(e, loss_v, np.mean(KLD_v))
