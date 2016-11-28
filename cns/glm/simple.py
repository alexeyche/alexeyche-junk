#!/usr/bin/env python

import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob

from tensorflow.python.ops import variable_scope as vs

def smooth(signal, sigma=0.01, filter_size=50):
	lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
	return np.convolve(lf_filter, signal, mode="same")


seq_size = 1000
filter_len = 100
filters_num = 200
strides = 1
factor = 0.1
dt = 1.0/1000.0
N = 1
lrate = 1e-03
epochs = 1000

input = tf.placeholder(tf.float32, shape=(1, seq_size, 1, 1), name="Input")

filter = vs.get_variable("EncodingFilter", [filter_len, 1, 1, filters_num], 
	initializer=tf.uniform_unit_scaling_initializer(factor=factor))

mu0 = vs.get_variable("mu", [N], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))

# W0 = vs.get_variable("W0", [filters_num, N], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))

target_spikes = tf.placeholder(tf.float32, shape=(seq_size,), name="Target")

conv_out = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='SAME')

conv_out = tf.squeeze(conv_out)

# elem = math_ops.matmul(conv_out, W0) + mu0
# lamb = tf.exp(elem)


elem = tf.reduce_sum(conv_out, 1) + mu0
lamb = tf.exp(elem)

spikes = tf.select(lamb * dt > tf.random_uniform([seq_size]), tf.ones([seq_size]), tf.zeros([seq_size]))


nlog_ll = - (tf.reduce_sum(target_spikes * elem) - dt * tf.reduce_sum(lamb))


# optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.RMSPropOptimizer(lrate)
optimizer = tf.train.AdagradOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)

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
	conv_out_v, lamb_v, spikes_v, nlog_ll_v, grads_v, _ = sess.run(
	    [
	        conv_out,
	        lamb,
	        spikes,
	        nlog_ll,
	        grads,
			apply_grads
	    ], 
	    {
	        input: inputs_v.reshape(1, seq_size, 1, 1),
	        target_spikes: target_spikes_v
	    }
	)
	loss = np.sum((smooth(spikes_v) - smooth(target_spikes_v))**2)
	print "Epoch {}, nlog ll {}, loss {}".format(ep, nlog_ll_v, loss)


