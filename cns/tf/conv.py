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

from conv_lib import SparseAcoustic
from env import Env

def generate_gammatone_dictionary(l, size, low_f=10, high_f=1000):
    def gammatone(t, f, phase, ampl, n, b):
        return ampl * t ** (n-1) * np.exp(-2.0 * np.pi * b * t) * np.cos(2.0 * np.pi * f * t + phase)
    gammatone_filters = np.zeros((l, size))

    t = np.linspace(0, 0.05, l)
    freqs = np.linspace(low_f, high_f, size)

    for fi, freq in enumerate(freqs):
        gammatone_filters[:, fi] = gammatone(t, freq, 0.0, 1.0, 2.0, 20.0)

    return gammatone_filters/np.sqrt(np.sum(gammatone_filters ** 2, 0))


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

env = Env("piano")

data_source = []
for f in sorted(os.listdir(env.dataset())):
    if f.endswith(".wav"):
        data_source.append(env.dataset(f))

model_fname = env.run("model.ckpt")

batch_size = 30000
L = 100
filters_num = 150
target_sr = 3000
gamma = 1e-03
epochs = 1000
lrate = 1e-03
k = 2 # filter strides
avg_size = 20
sel = None

input = tf.placeholder(tf.float32, shape=(1, batch_size, 1, 1), name="Input")


# init = lambda shape, dtype: generate_dct_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)
# init = lambda shape, dtype: np.reshape(-np.sqrt(3) / np.sqrt(L) + np.random.random((L, filters_num)) * 2.0*np.sqrt(3) / np.sqrt(L), (L, 1, 1, filters_num))
init = lambda shape, dtype: generate_gammatone_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)


filter = vs.get_variable("W", [L, 1, 1, filters_num], initializer=init)            
bias = vs.get_variable("b", [filters_num], initializer=lambda shape, dtype: np.zeros(filters_num))
recov_filter = vs.get_variable("Wr", [L, 1, 1, filters_num], initializer=init)

hidden = tf.nn.conv2d(input, filter, strides=[1, k, 1, 1], padding='VALID')
hidden_t = tf.pow(tf.nn.relu(hidden), 2.0)
# hidden_t = tf.nn.relu(hidden, bias)

hidden_p = tf.nn.avg_pool(hidden_t, [1, avg_size, 1, 1], strides = [1,1,1,1], padding='SAME')
# hidden_p = hidden_t
# hidden_t = tf.pow(hidden, 2.0)

# hidden_t = tf.abs(hidden)
hidden_p = hidden_p/tf.maximum(tf.reduce_max(hidden_p, [1]), tf.constant(1e-10))
# hidden_p = tf.nn.l2_normalize(hidden_p, dim=1)

output = tf.nn.conv2d_transpose(hidden_p, recov_filter, output_shape = (1, batch_size, 1, 1), strides=[1, k, 1, 1], padding='VALID')
output = tf.nn.l2_normalize(output, dim=1)


cost = tf.nn.l2_loss(output - input) + gamma * tf.reduce_mean(hidden_t)  #+ 0.01 * gamma * tf.reduce_sum(hidden_t)

optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.RMSPropOptimizer(lrate)
tvars = tf.trainable_variables()
grads = tf.gradients(cost, tvars)
train_step = optimizer.apply_gradients(zip(grads, tvars))



sess = tf.Session()
saver = tf.train.Saver()
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.initialize_all_variables())

def read_song(source_id):
    song_data_raw, source_sr = lr.load(data_source[source_id])
    song_data = lr.resample(song_data_raw, source_sr, target_sr, scale=True)
    song_data = song_data[:song_data.shape[0]/10]
    song_data, data_denom = norm(song_data)
    
    return song_data, source_sr, data_denom

def roll_around(data, collect_output=False):
    mc = []
    
    output_data = []
    hidden_data = []
    zero_hidden = set(xrange(filters_num))
    for id_start in xrange(0, data.shape[0], batch_size):
        data_slice = data[id_start:min(id_start+batch_size, data.shape[0])]
        
        batch_data = np.zeros(batch_size)
        batch_data[:len(data_slice)] = data_slice
        
        out_v, h_v_raw, h_v, filter_v, rfilter_v, bias_v, cost_v, _ = sess.run(
            [output, hidden_t, hidden_p, filter, recov_filter, bias, cost, train_step], {
                input: batch_data.reshape(1, batch_size, 1, 1)
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
            output_data.append(out_v.reshape(batch_size))
        mc.append(np.mean(cost_v))
    mean_cost = sum(mc)/len(mc)
    sparsity = float(len(zero_hidden))/filters_num

    if collect_output:
        return mean_cost, sparsity, filter_v, rfilter_v, bias_v, np.concatenate(hidden_data), np.concatenate(output_data)
    return mean_cost, sparsity, filter_v, rfilter_v, bias_v

def restore_hidden(recov_filter, hidden_data, batch_size=30000):
    output_data = []
    for id_start in xrange(0, hidden_data.shape[0], batch_size):
        data_slice = hidden_data[id_start:min(id_start+batch_size, hidden_data.shape[0])]
        data_slice[np.where(data_slice < 1e-02)] = 0.0
        
        hidden_input = tf.placeholder(tf.float32, shape=(1, data_slice.shape[0], 1, data_slice.shape[1]), name="Input")
        recov_filter_input = tf.placeholder(
            tf.float32, 
            shape=(recov_filter.shape[0], 1, 1, recov_filter.shape[1]), 
            name="RecovFilter"
        )
        
        output = tf.nn.conv2d_transpose(
            hidden_input, 
            recov_filter_input, 
            output_shape = (1, k*data_slice.shape[0]+recov_filter.shape[0]-1, 1, 1), 
            strides=[1, k, 1, 1], 
            padding='VALID'
        )
        output = tf.nn.l2_normalize(output, dim=1)
        out_v = sess.run([output], {
            recov_filter_input: recov_filter.reshape(recov_filter.shape[0], 1, 1, recov_filter.shape[1]), 
            hidden_input: data_slice.reshape(1, data_slice.shape[0], 1, data_slice.shape[1])
        })
        output_data.append(out_v[0].reshape(out_v[0].shape[1]))
    return np.concatenate(output_data)

# source_id = 0
# song_data, source_sr, data_denom = read_song(source_id)
# data_resample = lr.resample(song_data, target_sr, source_sr, scale=True)
# lr.output.write_wav(env.result("{}_resample_test.wav".format(source_id)), data_resample, source_sr)
dataset = []
for source_id in xrange(len(data_source)):
    if not sel is None and not source_id in sel:
        continue
    song_data, _, _     = read_song(source_id)
    dataset.append(song_data)

for e in xrange(epochs):
    mc, sp = [], []
    for data in dataset:
        mean_cost, sparsity, filter_v, rfilter_v, bias_v = roll_around(song_data)
        mc.append(mean_cost)    
        sp.append(sparsity)

    print "Epoch {}, cost {}, sparsity {}".format(e, sum(mc)/len(mc), sum(sp)/len(sp))

if epochs > 0:
    for source_id in xrange(len(data_source)):
        if not sel is None and not source_id in sel:
            continue
        
        song_data, source_sr, data_denom = read_song(source_id)
            
        _, sparsity, filter_v, rfilter_v, bias_v, hidden_final, _ = roll_around(song_data, True)
        
        hidden_final[np.where(hidden_final < 1e-05)] = 0.0

        sa = SparseAcoustic(hidden_final, data_denom)
        sa.serialize(env.dataset("{}_sparse_acoustic_data.dump".format(source_id)))

        out_final = restore_hidden(rfilter_v.reshape(L, filters_num), hidden_final)

        out_final *= data_denom

        data_recov = lr.resample(out_final, target_sr, source_sr, scale=True)
        lr.output.write_wav(env.result("{}_recovery.wav".format(source_id)), data_recov, source_sr)
    
    print "Saving in {}".format(saver.save(sess, model_fname))
else:
    rfilter_v = sess.run(recov_filter)

    for source_id in xrange(len(data_source)):
        if not sel is None and not source_id in sel:
            continue
        
        _, source_sr, _ = read_song(source_id)
        src = env.run("{}_nn_recovery.dump".format(source_id))
        dst = env.result("{}_nn_recovery.wav".format(source_id))
        if os.path.exists(src):
            print "Recovering {}".format(src)
            sa = SparseAcoustic.deserialize(src)

            out_final = restore_hidden(rfilter_v.reshape(L, filters_num), np.asarray(sa.data.todense()))

            out_final *= sa.data_denom
            data_recov = lr.resample(out_final, target_sr, source_sr, scale=True)
            lr.output.write_wav(dst, data_recov, source_sr)


# 
nf = 5
plt.figure(1)
plt.subplot(nf,1,1)
plt.imshow(filter_v.reshape(L, filters_num))
plt.subplot(nf,1,2)
plt.imshow(rfilter_v.reshape(L, filters_num))
plt.subplot(nf,1,3)
plt.plot(bias_v)
plt.subplot(nf,1,4)
plt.imshow(hidden_final[5000:10000,:].T)
plt.subplot(nf,1,5)
plt.plot(hidden_final[:, np.argmax(np.mean(hidden_final, 0))])
plt.show()


# plt.figure(1)
# plt.subplot(2,1,1)
# plt.plot(hidden_final[:,22])
# plt.subplot(2,1,2)
# plt.plot(np.exp(250.0*hidden_final[:,22]) - 1.0); plt.show()
# plt.show()