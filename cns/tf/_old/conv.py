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

from conv_lib import SparseAcoustic, norm
from conv_model import ConvModel, restore_hidden
from env import Env


env = Env("piano")

data_source = []
for f in sorted(os.listdir(env.dataset())):
    if f.endswith(".wav"):
        data_source.append(env.dataset(f))

model_fname = env.run("model.ckpt")

batch_size = 30000
L = 150
filters_num = 100
target_sr = 3000
gamma = 1e-03
epochs = 2000
lrate = 1e-04
k = 8 # filter strides
avg_size = 5
sel = None


cm = ConvModel(batch_size, L, filters_num, k, avg_size, lrate, gamma)


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
        out_v, h_v, filter_v, rfilter_v, bias_v, cost_v = cm.run(sess, batch_data)
        
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
        
        # hidden_final[np.where(hidden_final < 1e-05)] = 0.0

        sa = SparseAcoustic(hidden_final, data_denom)
        sa.serialize(env.dataset("{}_sparse_acoustic_data.dump".format(source_id)))

        out_final = restore_hidden(sess, rfilter_v.reshape(L, filters_num), hidden_final, k)

        out_final *= data_denom

        data_recov = lr.resample(out_final, target_sr, source_sr, scale=True)
        lr.output.write_wav(env.result("{}_recovery.wav".format(source_id)), data_recov, source_sr)
    
    print "Saving in {}".format(saver.save(sess, model_fname))
    rfilter_v = sess.run(cm.recov_filter)
    np.save(open(env.run("recov_filter.pkl"), "w"), rfilter_v)
# else:
#     source_sr = 22000
#     src = env.run("nn_dream.dump")
#     dst = env.result("nn_dream.wav")
#     if os.path.exists(src):
#         print "Recovering {}".format(src)
#         sa = SparseAcoustic.deserialize(src)

#         out_final = restore_hidden(rfilter_v.reshape(L, filters_num), np.asarray(sa.data.todense()))

#         out_final *= sa.data_denom
#         data_recov = lr.resample(out_final, target_sr, source_sr, scale=True)
#         lr.output.write_wav(dst, data_recov, source_sr)

    # for source_id in xrange(len(data_source)):
    #     if not sel is None and not source_id in sel:
    #         continue
        
    #     _, source_sr, _ = read_song(source_id)
    #     src = env.run("{}_nn_recovery.dump".format(source_id))
    #     dst = env.result("{}_nn_recovery.wav".format(source_id))
    #     if os.path.exists(src):
    #         print "Recovering {}".format(src)
    #         sa = SparseAcoustic.deserialize(src)

    #         out_final = restore_hidden(rfilter_v.reshape(L, filters_num), np.asarray(sa.data.todense()))

    #         out_final *= sa.data_denom
    #         data_recov = lr.resample(out_final, target_sr, source_sr, scale=True)
    #         lr.output.write_wav(dst, data_recov, source_sr)


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