#!/usr/bin/env python

import os

import numpy as np
from matplotlib import pyplot as plt

import librosa as lr
from os.path import join as pj

import tensorflow as tf
from tensorflow.python.ops import math_ops

from env import current as env


filters_num = 500
filter_size = 100
batch_size = 100000
target_sr = 3000
sigma_zca = 1e-05

def colnorm(f):
	return f.T/np.sqrt(np.sum(f ** 2, 1)).T


def form_batch(it, data, batch_size, filter_size):
    if it == batch_size:
        it -= filter_size
    data_iter_start = it

    Rn_v = np.zeros((batch_size, filter_size))
    scales = np.ones(batch_size)
    for bi in xrange(batch_size):
        Rn_v[bi, :] = data[it:(it + filter_size)]
        
        it += 1
        if it + filter_size >= data.shape[0]:
            break
    return Rn_v, it

def calc_mean_and_var(data, batch_size, window_size):
	it = 0
	x_means = []
	while it + window_size < data.shape[0]:
		Rn_v, it = form_batch(it, data, batch_size, window_size)
		
		x_means.append(np.mean(Rn_v, 0))

	x_mean = np.sum(x_means, 0)/len(x_means)

	it = 0
	x_vars = []
	while it + window_size < data.shape[0]:
		Rn_v, it = form_batch(it, data, batch_size, window_size)

		x_vars.append(np.mean(np.square(Rn_v - x_mean), 0))

	x_var = np.sum(x_vars, 0)/len(x_vars)
	return x_mean, x_var




df = env.run("test_data.pkl")
fname = env.dataset([f for f in os.listdir(env.dataset()) if f.endswith(".wav")][0])

if not os.path.exists(df):
    song_data_raw, source_sr = lr.load(fname)
    print "Got sampling rate {}, resampling to {} ...".format(source_sr, target_sr)
    data = lr.resample(song_data_raw, source_sr, target_sr, scale=True)
    # data = song_data[:30000,]

    np.save(open(df, "w"), data)
else:
    data = np.load(open(df))


x_mean, x_var = calc_mean_and_var(data, batch_size, filter_size)

dd = form_batch(0, data, data.shape[0], filter_size)[0]


dd = (dd - x_mean)/np.sqrt(x_var + 1e-05)
x_cov = np.cov(dd.T)
w, V = np.linalg.eig(x_cov)
D = np.diag(w)


zcaM = np.dot(np.dot(V, 1.0/np.sqrt(D + sigma_zca)), V.T)





#dd = np.dot(zcaM, )





filters = tf.placeholder(tf.float32, shape=(filters_num, filter_size), name="Filters")
Rn = tf.placeholder(tf.float32, shape=(batch_size, filter_size), name="Rn")

prods = math_ops.matmul(filters, tf.transpose(Rn))






filters_v = colnorm(np.random.randn(filters_num, filter_size))
