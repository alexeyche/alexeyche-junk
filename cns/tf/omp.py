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



ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

res_dir = pj(os.environ["HOME"], "Music", "ml", "impro_res")
if not os.path.exists(res_dir):
    os.makedirs(res_dir)


filters_num = 1000
filter_size = 10000
jobs = 1
batch_size = 100000
learning_rate = 0.001
epochs = 1
learning = False
target_sr = 3000
source_id = 1

filters = np.random.randn(filters_num, filter_size)
filters = (filters.T/np.sqrt(np.sum(filters ** 2, 1))).T
filters_init = filters.copy()


data, source_sr = lr.load(data_source[source_id])
data = lr.resample(data, source_sr, target_sr, scale=True)
data = data[10000:100000]

data_test = lr.resample(data, target_sr, source_sr, scale=True)
lr.output.write_wav(pj(res_dir, "{}_test_resample.wav".format(source_id)), data_test, source_sr)


from sklearn.linear_model import OrthogonalMatchingPursuit
omp = OrthogonalMatchingPursuit(n_nonzero_coefs=int(0.1*filters_num))
r = omp.fit(filters.T, data[:filter_size])

plt.plot(data[:filter_size])
plt.plot(np.dot(r.coef_.T, filters)) 
plt.show()