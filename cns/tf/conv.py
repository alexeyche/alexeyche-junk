#!/usr/bin/env python

import logging
import sys
import numpy as np
import os
from matplotlib import pyplot as plt
import librosa as lr
from os.path import join as pj

import tensorflow as tf

ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

res_dir = pj(os.environ["HOME"], "Music", "ml", "impro_res")
if not os.path.exists(res_dir):
    os.makedirs(res_dir)


w = 5000
L = 100
filters_num = 200

batch_size = 1

input = tf.placeholder(tf.float32, shape=(batch_size, w, 1, 1), name="Input")

delta = tf.placeholder(tf.float32, shape=(1, L+1, 1, 1), name="Delta")

filter = tf.placeholder(tf.float32, shape=(L, 1, 1, 1), name="Filter")
recov_filter = tf.placeholder(tf.float32, shape=(L, 1, 1, 1), name="RecovFilter")

op = tf.nn.conv2d(input, filter, strides=[1, 1, 1, 1], padding='VALID')

op_recov = tf.nn.conv2d_transpose(op, filter, output_shape = (batch_size, w, 1, 1), strides=[1, 1, 1, 1], padding='VALID')

sess = tf.Session()
sess.run(tf.initialize_all_variables())


input_v = np.zeros(w)
input_v[50] = 1.0


delta_v = np.zeros(L+1)
delta_v[-1] = 1.0

filters_v = generate_dct_dictionary(filters_num, filter_size)

op_v, op_recov_v, recov_filter_v = sess.run(
	[op, op_recov, recov_filter], {
		input: input_v.reshape(batch_size, w, 1, 1),
		filter: filter_v.reshape(L, 1, 1, 1),
        delta: delta_v.reshape(1, L+1, 1, 1)
	}
)
