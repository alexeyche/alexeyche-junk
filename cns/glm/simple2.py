#!/usr/bin/env python

import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob

from tensorflow.python.ops import math_ops
from tensorflow.python.ops import variable_scope as vs

def smooth(signal, sigma=0.01, filter_size=50):
	lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
	return np.convolve(lf_filter, signal, mode="same")


seq_size = 1000
filter_len = 100
filters_num = 100
strides = 1
factor = 0.1
dt = 1.0/1000.0
N = 100
lrate = 1e-04
epochs = 1000

input = tf.placeholder(tf.float32, shape=(1, seq_size, 1, 1), name="Input")

filter0 = vs.get_variable("EncodingFilter0", [filter_len, 1, 1, filters_num], 
	initializer=tf.uniform_unit_scaling_initializer(factor=factor))
filter1 = vs.get_variable("EncodingFilter1", [filter_len, 1, N, filters_num], 
	initializer=tf.uniform_unit_scaling_initializer(factor=factor))

mu0 = vs.get_variable("mu0", [N], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))
W0 = vs.get_variable("W0", [filters_num, N], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))
mu1 = vs.get_variable("mu1", [1], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))
W1 = vs.get_variable("W1", [filters_num, 1], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))



target_spikes = tf.placeholder(tf.float32, shape=(seq_size,), name="Target")

conv_out0 = tf.nn.conv2d(input, filter0, strides=[1, strides, 1, 1], padding='SAME')
conv_out0 = tf.squeeze(conv_out0)

elem0 = math_ops.matmul(conv_out0, W0) + mu0
lamb0 = tf.exp(elem0)

# spikes0 = tf.select(lamb0 * dt > tf.random_uniform([seq_size, N]), tf.ones([seq_size, N]), tf.zeros([seq_size, N]))
spikes0 = lamb0
spikes0 = tf.expand_dims(spikes0, 0)
spikes0 = tf.expand_dims(spikes0, 2)


conv_out1 = tf.nn.conv2d(spikes0, filter1, strides=[1, strides, 1, 1], padding='SAME')
conv_out1 = tf.squeeze(conv_out1)


elem = math_ops.matmul(conv_out1, W1) + mu1
lamb = tf.exp(elem)

elem = tf.squeeze(elem)

nlog_ll = - (tf.reduce_sum(target_spikes * elem) - dt * tf.reduce_sum(lamb))


# optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.RMSPropOptimizer(lrate)
# optimizer = tf.train.AdagradOptimizer(lrate)
optimizer = tf.train.GradientDescentOptimizer(lrate)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(nlog_ll, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

apply_grads = optimizer.apply_gradients(zip(grads, tvars))



sess = tf.Session()
sess.run(tf.initialize_all_variables())


inputs_v = np.random.randn(seq_size)
target_spikes_v = np.zeros(seq_size)

target_spikes_v[240] = 1.0
target_spikes_v[250] = 1.0
target_spikes_v[260] = 1.0
target_spikes_v[490] = 1.0
target_spikes_v[500] = 1.0
target_spikes_v[510] = 1.0
target_spikes_v[740] = 1.0
target_spikes_v[750] = 1.0
target_spikes_v[760] = 1.0



for ep in xrange(epochs):
	conv_out_v, lamb0_v, spikes_v, lamb1_v, elem_v, nlog_ll_v, grads_v, _ = sess.run(
	    [
	        conv_out1,
	        lamb0,
	        spikes0,
	        lamb,
	        elem,
	        nlog_ll,
	        grads,
			apply_grads
	    ], 
	    {
	        input: inputs_v.reshape(1, seq_size, 1, 1),
	        target_spikes: target_spikes_v
	    }
	)
	# loss = np.sum((smooth(np.squeeze(spikes_v)) - smooth(target_spikes_v))**2)
	print "Epoch {}, loss {}".format(ep, nlog_ll_v)


