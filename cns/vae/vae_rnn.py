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


input = tf.placeholder(tf.float32, shape=(1, seq_size, batch_size, 1), name="Input")

env = Env("vae_run")


init = lambda shape, dtype: generate_dct_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)

filter = tf.Variable(generate_dct_dictionary(L, filters_num).reshape(L, 1, 1, filters_num), name="Filter", dtype=tf.float32)

be0 = tf.Variable(np.zeros(h0_size), name="Bias")
We0 = tf.Variable(xavier_init(filters_num, h0_size), name="We0")

input_filtered = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='VALID')



h0 = tf.nn.relu(tf.matmul(input_filtered, We0) + be0)

sess = tf.Session()

model_fname = env.run("model.ckpt")
saver = tf.train.Saver()
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.initialize_all_variables())


input_v =  moving_average(np.random.randn(seq_size), 10).reshape(seq_size, batch_size, 1)


h0_v = sess.run(h0, {
	input: input_v
})

