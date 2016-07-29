#!/usr/bin/env python

import logging
import sys
import numpy as np
import os
from matplotlib import pyplot as plt
import librosa as lr
from os.path import join as pj

import tensorflow as tf
from tensorflow.python.ops import variable_scope as vs

def generate_dct_dictionary(l, size):
    p = np.asarray(xrange(l))
    filters = np.zeros((l, size))
    for fi in xrange(size):
        filters[:, fi] = np.cos((np.pi * (2 * fi + 1) * p)/(2*l))
        filters[0, fi] *= 1.0/np.sqrt(2.0)
        # filters[fi, 1:] *= np.sqrt(2/l) 
    return filters * np.sqrt(2.0/l)

def norm(data):
	data_denom = np.sqrt(np.sum(data ** 2))
	data = data/data_denom
	return data, data_denom

ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

res_dir = pj(os.environ["HOME"], "Music", "ml", "impro_res")
if not os.path.exists(res_dir):
    os.makedirs(res_dir)



batch_size = 10000
L = 100
filters_num = 500
target_sr = 3000
p = 0.001
thr = 0.1
gain = 10.0

input = tf.placeholder(tf.float32, shape=(1, batch_size, 1, 1), name="Input")


init = lambda shape, dtype: generate_dct_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)
filter = vs.get_variable("W", [L, 1, 1, filters_num], initializer=init)            
recov_filter = vs.get_variable("Wr", [L, 1, 1, filters_num], initializer=init)            

hidden = tf.nn.conv2d(input, filter, strides=[1, 1, 1, 1], padding='VALID')
hidden_t = tf.nn.relu(tf.exp(gain*hidden) - 1.0 - thr)
# hidden_t = tf.nn.l2_normalize(hidden_t, dim=1)
output = tf.nn.conv2d_transpose(hidden_t, recov_filter, output_shape = (1, batch_size, 1, 1), strides=[1, 1, 1, 1], padding='VALID')
output = tf.nn.l2_normalize(output, dim=1)


cost = tf.nn.l2_loss(output - input) 

optimizer = tf.train.AdamOptimizer(1e-03)
tvars = tf.trainable_variables()
grads = tf.gradients(cost, tvars)
train_step = optimizer.apply_gradients(zip(grads, tvars))


source_id = 1
song_data_raw, source_sr = lr.load(data_source[source_id])
song_data = lr.resample(song_data_raw, source_sr, target_sr, scale=True)

song_data, data_denom = norm(song_data)

sess = tf.Session()
sess.run(tf.initialize_all_variables())

def roll_around(data, collect_output=False):
	sum_cost = 0
	
	output_data = []
	hidden_data = []
	for id_start in xrange(0, data.shape[0], batch_size):
		data_slice = data[id_start:min(id_start+batch_size, data.shape[0])]
		
		batch_data = np.zeros(batch_size)
		batch_data[:len(data_slice)] = data_slice
		
		out_v, h_v, filter_v, rfilter_v, cost_v, _ = sess.run(
			[output, hidden_t, filter, recov_filter, cost, train_step], {
				input: batch_data.reshape(1, batch_size, 1, 1)
			}
		)

		if collect_output:
			hidden_data.append(h_v.reshape(h_v.shape[1], h_v.shape[3]))
			output_data.append(out_v.reshape(batch_size))
		sum_cost += cost_v
	
	mean_cost = sum_cost/(data.shape[0]/batch_size)

	if collect_output:
		return mean_cost, np.concatenate(hidden_data), np.concatenate(output_data)
	return mean_cost


for e in xrange(100):
	mean_cost = roll_around(song_data)
	print "Epoch {}, cost {}".format(e, mean_cost)


_, hidden_final, out_final = roll_around(song_data, True)

out_final *= data_denom

data_test = lr.resample(song_data*data_denom, target_sr, source_sr, scale=True)
lr.output.write_wav(pj(res_dir, "{}_resample.wav".format(source_id)), data_test, source_sr)

data_recov = lr.resample(out_final, target_sr, source_sr, scale=True)
lr.output.write_wav(pj(res_dir, "{}_recovery.wav".format(source_id)), data_recov, source_sr)
