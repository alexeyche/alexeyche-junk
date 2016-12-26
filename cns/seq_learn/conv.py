#!/usr/bin/env python

import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob

from tensorflow.python.ops import variable_scope as vs

import librosa as lr
import os
import scipy

from env import Env


def generate_dct_dictionary(l, size):
    p = np.asarray(xrange(l))
    filters = np.zeros((l, size))
    for fi in xrange(size):
        filters[:, fi] = np.cos((np.pi * (2 * fi + 1) * p)/(2*l))
        filters[0, fi] *= 1.0/np.sqrt(2.0)
        # filters[fi, 1:] *= np.sqrt(2/l) 
    return filters * np.sqrt(2.0/l)

def sigmoid(x):
    return 1.0/(1.0 +np.exp(-x))

def norm(data):
    data_denom = np.sqrt(np.sum(data ** 2))
    data = data/data_denom
    return data, data_denom

seq_size = 10000
strides = 1
L = 100
filters_num = 100
batch_size = 1
target_sr = 5000 
avg_size = 5
gamma = 1.0 
lrate = 1e-04
epochs = 300
collect_output = True    

input = tf.placeholder(tf.float32, shape=(1, seq_size, 1, 1), name="Input")

# init = lambda shape, dtype: generate_dct_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)
init = lambda shape, dtype: np.reshape(-np.sqrt(3) / np.sqrt(L) + np.random.random((L, filters_num)) * 2.0*np.sqrt(3) / np.sqrt(L), (L, 1, 1, filters_num))
# init = lambda shape, dtype: generate_gammatone_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)


filter = vs.get_variable("W", [L, 1, 1, filters_num], initializer=init)            
bias = vs.get_variable("b", [filters_num], initializer=lambda shape, dtype: np.zeros(filters_num))
recov_filter = vs.get_variable("Wr", [L, 1, 1, filters_num], initializer=init)

hidden = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='VALID')
# hidden_t = hidden
hidden_t = tf.pow(tf.nn.relu(hidden), 4.0)
# hidden_t = tf.pow(tf.nn.relu(tf.nn.bias_add(hidden, bias)), 2.0)

# hidden_p = tf.nn.avg_pool(hidden_t, [1, avg_size, 1, 1], strides = [1,1,1,1], padding='SAME')
hidden_p = hidden_t
# hidden_t = tf.pow(hidden, 2.0)

# hidden_t = tf.abs(hidden)
# hidden_p = hidden_p/tf.maximum(tf.reduce_max(hidden_p, [1]), tf.constant(1e-10))
hidden_p = tf.nn.l2_normalize(hidden_p, dim=1)

output = tf.nn.conv2d_transpose(hidden_p, recov_filter, output_shape = (1, seq_size, 1, 1), strides=[1, strides, 1, 1], padding='VALID')

output_n = tf.nn.l2_normalize(output, dim=1)
input_n = tf.nn.l2_normalize(input, dim=1)
cost = tf.nn.l2_loss(output_n - input_n) + gamma * tf.reduce_mean(hidden_t)  #+ 0.01 * gamma * tf.reduce_sum(hidden_t)

optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.RMSPropOptimizer(lrate)
tvars = tf.trainable_variables()
grads = tf.gradients(cost, tvars)
train_step = optimizer.apply_gradients(zip(grads, tvars))



# RUNNING 

env = Env("simple_test")

data_source = []
for f in sorted(os.listdir(env.dataset())):
    if f.endswith(".wav"):
        data_source.append(env.dataset(f))

def read_song(source_id):
    song_data_raw, source_sr = lr.load(data_source[source_id])
    song_data = lr.resample(song_data_raw, source_sr, target_sr, scale=True)
    song_data = song_data[:song_data.shape[0]/10]
    song_data, data_denom = norm(song_data)
    
    return song_data, source_sr, data_denom


data, source_sr, data_denom = read_song(0)

sess = tf.Session()

model_fname = env.run("model.ckpt")
saver = tf.train.Saver()
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.initialize_all_variables())

for e in xrange(epochs):
    mc = []

    output_data = []
    hidden_data = []
    zero_hidden = set(xrange(filters_num))
    for id_start in xrange(0, data.shape[0], seq_size):
        data_slice = data[id_start:min(id_start+seq_size, data.shape[0])]
        
        batch_data = np.zeros(seq_size)
        batch_data[:len(data_slice)] = data_slice
        
        out_v, in_v, h_v_raw, h_v, filter_v, rfilter_v, bias_v, cost_v, _ = sess.run(
            [output_n, input_n, hidden_t, hidden_p, filter, recov_filter, bias, cost, train_step], {
                input: batch_data.reshape(1, seq_size, 1, 1)
            }
        )
        h_v_raw = h_v_raw.reshape(h_v_raw.shape[1], h_v_raw.shape[3])
        h_v = h_v.reshape(h_v.shape[1], h_v.shape[3])
        
        # plt.figure(1)
        # plt.subplot(2,1,1)
        # plt.plot(h_v_raw[0:2000,21])
        # plt.subplot(2,1,2)
        # plt.plot(h_v[0:2000,21])
        # plt.show()
        
        zero_hidden &= set(np.where(np.mean(h_v, 0) == 0.0)[0])

        if collect_output:
            hidden_data.append(h_v)
            output_data.append(out_v.reshape(seq_size))
        mc.append(np.mean(cost_v))
    mean_cost = sum(mc)/len(mc)
    sparsity = float(len(zero_hidden))/filters_num

    print "Epoch {}, cost {}, sparsity {}".format(e, mean_cost, sparsity)
