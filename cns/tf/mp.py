#!/usr/bin/env python

import logging
import sys
import numpy as np
import os
from matplotlib import pyplot as plt
import librosa as lr
from os.path import join as pj
from multiprocessing import Process
from multiprocessing import Queue
import time
from Queue import Empty

import tensorflow as tf
from tensorflow.python.ops import math_ops

import cPickle as pkl

from mp_lib import SpikeRecords
from mp_lib import run_on_batch
from mp_lib import restore
from mp_lib import gammatone
from mp_lib import generate_dct_dictionary



ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

res_dir = pj(os.environ["HOME"], "Music", "ml", "impro_res")
if not os.path.exists(res_dir):
    os.makedirs(res_dir)



filters_num = 100
filter_size = 200
jobs = 1
batch_size = 100000
threshold = 0.2
match_iterations = 5
learning_rate = 0.001
momentum = 0.0
epochs = 1
learning = False
target_sr = 3000


filters = np.zeros((filters_num, filter_size))
dfilters = np.zeros((filters_num, filter_size))

# t = np.linspace(0, 0.05, filter_size)
# freqs = np.linspace(10, 1000, filters_num)

# for fi, freq in enumerate(freqs):
#     filters[fi, :] = gammatone(t, freq, 0.0, 1.0, 2.0, 20.0)

# subf = 10
# subfilters = generate_dct_dictionary(filters_num/subf, filter_size)
# filters_list = []
# for factor in np.linspace(0.1, 2.0, subf):
#     print factor
#     filters_list.append(subfilters * factor)

# filters = np.concatenate(filters_list)

filters = generate_dct_dictionary(filters_num, filter_size)

filters = (filters.T/np.sqrt(np.sum(filters ** 2, 1))).T
filters_init = filters.copy()

use_gpu = True
filters_gpu = tf.placeholder(tf.float32, shape=filters.shape, name="Filters")
Rn_gpu = tf.placeholder(tf.float32, shape=(batch_size, filter_size), name="Rn")
prods_gpu = math_ops.matmul(filters_gpu, tf.transpose(Rn_gpu))
gpu_session = tf.Session()    

assert not use_gpu or jobs == 1, "Can't use gpu with multiple processes"

for source_id, source_filename in enumerate(data_source):
    data, source_sr = lr.load(source_filename)
    data = lr.resample(data, source_sr, target_sr, scale=True)

    # data_test = lr.resample(data, target_sr, source_sr, scale=True)
    # lr.output.write_wav("/home/alexeyche/Music/ml/test.wav", data_test, source_sr)

    data_denom = np.sqrt(np.sum(data ** 2))
    data = data/data_denom
    data = np.concatenate([data, np.zeros(filter_size)]) 

    print "Source with id {} and file {}".format(source_id, source_filename)
    processes = []
    records = []

    def sync(wait_all=True):
        global dfilters, records, processes, filters
        while len(processes)>0:
            ids_to_delete = set()
            for pi, (p, q) in enumerate(processes):
                try:
                    batch_records, dfilters_from_batch = q.get(timeout=0.5)
                    if p:
                        p.terminate()
                        p.join()
                    ids_to_delete.add(pi)
                    
                    if len(batch_records)>0:
                        if learning:
                            dfilters = dfilters * momentum + dfilters_from_batch
                            filters += dfilters * learning_rate
                        records += batch_records
                    
                    if not wait_all:
                        processes = [ p for pi, p in enumerate(processes) if not pi in ids_to_delete ]
                        return

                except Empty:
                    pass
            processes = [ p for pi, p in enumerate(processes) if not pi in ids_to_delete ]
                                                        


    data_iter = 0
    while data_iter + filter_size < data.shape[0]:
        if data_iter == batch_size:
            data_iter -= filter_size
        data_iter_start = data_iter

        Rn = np.zeros((batch_size, filter_size))
        for bi in xrange(batch_size):
            Rn[bi, :] = data[data_iter:(data_iter + filter_size)]     
            Rn_sum = np.sum(Rn[bi, :] ** 2)
            if Rn_sum > 1e-05:
                Rn[bi, :] = Rn[bi, :] / np.sqrt(Rn_sum)
            
            data_iter += 1
            if data_iter + filter_size >= data.shape[0]: 
                break

        queue = Queue()
        if use_gpu:
            run_on_batch(
                data_iter_start, 
                Rn, 
                data[data_iter_start:(data_iter_start+batch_size)], 
                filters, 
                match_iterations, 
                threshold,
                learning,
                lambda filters, Rn: gpu_session.run([prods_gpu], {filters_gpu: filters, Rn_gpu: Rn})[0],
                queue
            )
            processes.append((None, queue))
            sync(wait_all=False)
        else:
            process = Process(
                target = run_on_batch, 
                args = (
                    data_iter_start, 
                    Rn, 
                    data[data_iter_start:(data_iter_start+batch_size)], 
                    filters, 
                    match_iterations, 
                    threshold,
                    learning,
                    None,
                    queue
                )
            )
            process.start()
            processes.append((process, queue))
            if len(processes) == jobs:
                sync(wait_all=False)
    sync(wait_all=True)
    
    spike_records = SpikeRecords(records, filters_num, threshold, data_denom, target_sr)
    data_restored, sr = spike_records.get_waveform(filters)
    
    restored_file = pj(res_dir, "{}_input_restored.wav".format(source_id))
    print "Saving restored file in {}".format(restored_file)
    data_resampled = lr.resample(data_restored, sr, source_sr, scale=True)
    lr.output.write_wav(restored_file, data_resampled, source_sr)
    
    spikes_file = pj(ds_dir, "{}_spikes.pkl".format(source_id))
    print "Saving spikes in {}".format(spikes_file)

    pkl.dump(spike_records, open(spikes_file, "wb"), protocol=2)
    print "Done"

filters_file = pj(ds_dir, "filters.pkl") 
print "Saving filters in {}".format(filters_file)
np.save(open(filters_file, "w"), filters)

recs = np.asarray(records)

plt.figure(1)
plt.subplot(2,1,1)
plt.scatter(recs[:, 0], recs[:, 1])
plt.subplot(2,1,2)
plt.plot(data*data_denom)
plt.plot(data_restored)
plt.show()


