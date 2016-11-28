#!/usr/bin/env python

import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob
import librosa as lr
import os
from env import Env

from tensorflow.python.ops import math_ops
from tensorflow.python.ops import variable_scope as vs

def smooth(signal, sigma=0.01, filter_size=50):
	lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
	return np.convolve(lf_filter, signal, mode="same")

def norm(data):
    data_denom = np.sqrt(np.sum(data ** 2))
    data = data/data_denom
    return data, data_denom


env = Env("piano")

#########################################
# MODEL

seq_size = 30000
filter_len = 100
filters_num = 100
strides = 1
factor = 0.1
dt = 1.0/1000.0
N = 100
lrate = 1e-04
epochs = 100
target_sr = 5000
batch_size = 1

init = lambda shape, dtype: np.reshape(-np.sqrt(3) / np.sqrt(shape[0]) + np.random.random((shape[0], shape[3])) * 2.0*np.sqrt(3) / np.sqrt(shape[0]), (shape[0], 1, 1, shape[3]))
input = tf.placeholder(tf.float32, shape=(1, seq_size, 1, 1), name="Input")
target = tf.placeholder(tf.float32, shape=(1, seq_size, 1, 1), name="Target")

filter = vs.get_variable("EncodingFilter", [filter_len, 1, 1, filters_num], 
	initializer=tf.uniform_unit_scaling_initializer(factor=factor))

recov_filter = vs.get_variable("DecodingFilter", [filter_len, 1, N, 1], 
	initializer=tf.uniform_unit_scaling_initializer(factor=factor))

mu0 = vs.get_variable("mu", [N], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))
W0 = vs.get_variable("W0", [filters_num, N], initializer=tf.uniform_unit_scaling_initializer(factor=1.0))

target_spikes = tf.placeholder(tf.float32, shape=(seq_size,), name="Target")

conv_out = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='SAME')

conv_out = tf.squeeze(conv_out)

elem = math_ops.matmul(conv_out, W0) + mu0
lamb = tf.exp(elem)

spikes = tf.select(lamb * dt > tf.random_uniform([seq_size, N]), tf.ones([seq_size, N]), tf.zeros([seq_size, N]))

# spikes = lamb

spikes = tf.expand_dims(spikes, 0)
spikes = tf.expand_dims(spikes, 2)

output = tf.nn.conv2d(spikes, recov_filter, strides=[1, 1, 1, 1], padding='SAME')

# target_norm = tf.nn.l2_normalize(target, dim=1)
# output_norm = tf.nn.l2_normalize(output, dim=1)

cost = tf.nn.l2_loss(output - target) / seq_size / batch_size


# nlog_ll = - (tf.reduce_sum(target_spikes * elem) - dt * tf.reduce_sum(lamb))




# optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.RMSPropOptimizer(lrate)
optimizer = tf.train.AdagradOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)

apply_grads = optimizer.minimize(cost)

# tvars = tf.trainable_variables()
# grads_raw = tf.gradients(cost, tvars)
# grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

# apply_grads = optimizer.apply_gradients(zip(grads, tvars))

##################################
# DATA

fname = env.dataset([f for f in os.listdir(env.dataset()) if f.endswith(".wav")][0])
df = env.run("test_data.pkl")

if not os.path.exists(df):
    song_data_raw, source_sr = lr.load(fname)
    print "Got sampling rate {}, resampling to {} ...".format(source_sr, target_sr)
    song_data = lr.resample(song_data_raw, source_sr, target_sr, scale=True)
    song_data = song_data[:30000,]

    np.save(open(df, "w"), song_data)
else:
    song_data = np.load(open(df))

inputs_v, data_denom = norm(song_data)




##################################
# EVALUATION

sess = tf.Session()
sess.run(tf.initialize_all_variables())



for ep in xrange(epochs):
	conv_out_v, lamb_v, spikes_v, cost_v, _ = sess.run(
	    [
	        conv_out,
	        lamb,
	        spikes,
	        cost,
	        # grads,
			apply_grads
	    ], 
	    {
	        input: inputs_v.reshape(1, seq_size, 1, 1),
	        target: inputs_v.reshape(1, seq_size, 1, 1)
	    }
	)
	print "Epoch {}, cost {}".format(ep, cost_v)


