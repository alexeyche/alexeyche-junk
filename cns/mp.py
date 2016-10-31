#!/usr/bin/env python

import logging
import sys
import ctypes as ct
import numpy as np
from numpy.ctypeslib import ndpointer
import os
from matplotlib import pyplot as plt
import librosa as lr
from os.path import join as pj


class FilterMatch(ct.Structure):
    _fields_ = [
        ("Fi", ct.c_uint),
        ("S", ct.c_double),
        ("T", ct.c_uint),
    ]

    def __init__(self, *args):
        if len(args) > 0:
            self.Fi = args[0]
            self.S = args[1]
            self.T = args[2]
            


lib_dir = "/home/alexeyche/prog/alexeyche-junk/cns/tf"

_mpl_lib = np.ctypeslib.load_library('libacousticfeatures', lib_dir)

_mpl_lib.run_mpl.restype = None
_mpl_lib.run_mpl.argtypes = [
    ndpointer(ndim=1, flags='CONTIGUOUS,ALIGNED'),  # data
    ct.c_uint,  # data length
    ndpointer(ndim=2, flags='CONTIGUOUS,ALIGNED'),  # filters
    ct.c_uint, ct.c_uint,  # filters number, size
    ct.c_double, # threshold
    ct.c_bool,   # learn flag
    ct.c_double, # learning rate
    ct.c_uint,  # learn iterations
    ct.c_double, # momentum
    ct.c_uint,  # batch size
    ct.c_uint,  # jobs
    ct.POINTER(ct.POINTER(FilterMatch)),   # return filter matches
    ct.POINTER(ct.c_uint),                 # filter matches array size
    ct.POINTER(ct.POINTER(ct.c_double)),   # residual
    ct.POINTER(ct.c_uint),                 # residual length
    ct.POINTER(ct.c_double)                # mean loss
]
_mpl_lib.free_matches.restype = None
_mpl_lib.free_matches.argtypes = [
    ct.POINTER(ct.POINTER(FilterMatch))
]
_mpl_lib.free_double.restype = None
_mpl_lib.free_double.argtypes = [
    ct.POINTER(ct.POINTER(ct.c_double))
]

_mpl_lib.restore.restype = None
_mpl_lib.restore.argtypes = [
    ct.POINTER(FilterMatch),                            # filter matches
    ct.c_uint,                                          # filter matches size
    ndpointer(ndim=2, flags='CONTIGUOUS,ALIGNED'),      # filters
    ct.c_uint, ct.c_uint,                               # filters number, size
    ct.POINTER(ct.POINTER(ct.c_double)),                # result signal
    ct.POINTER(ct.c_uint)                               # result signal size
]




ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_list_files = []
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".npy"):
        data_list_files.append(pj(ds_dir, f))
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

res_dir = pj(os.environ["HOME"], "Music", "ml", "impro_res")
if not os.path.exists(res_dir):
    os.makedirs(res_dir)

source_id = 1

y, sr = lr.load(data_source[source_id])
data = y[10000:15000]




# lib_dir = os.path.dirname(os.path.realpath(__file__))

filters_num = 50
filter_size = 100
jobs = 1
batch_size = 5000
threshold = 0.05
learn_iterations = 100
learning_rate = 0.00001
learn = True
momentum = 0.9

# data = np.sin(np.linspace(0, 50000.0, 100000))
# data = np.random.randn(100000)
# data = data/np.sqrt(np.sum(data ** 2.0))
np.random.seed = 10
filts_init = np.random.randn(filters_num, filter_size)
filts = filts_init.copy()

for epoch in xrange(1):
    matches_arr = ct.POINTER(FilterMatch)()
    matches_size = ct.c_uint(0)

    residual_arr = ct.POINTER(ct.c_double)()
    residual_size = ct.c_uint(0)
    mean_loss = ct.c_double(0)

    _mpl_lib.run_mpl(
        data, data.shape[0],
        filts, filters_num, filter_size, 
        threshold,
        learn,
        learning_rate,
        learn_iterations,
        momentum,
        batch_size, jobs, 
        ct.byref(matches_arr), ct.byref(matches_size),
        ct.byref(residual_arr), ct.byref(residual_size),
        ct.byref(mean_loss),
    )
    print "Epoch {}, mean loss {}".format(epoch, mean_loss.value)
    matches = np.asarray([ (matches_arr[mi].Fi, matches_arr[mi].S, matches_arr[mi].T) for mi in xrange(matches_size.value) ]).T
    residual = np.asarray([ residual_arr[ri] for ri in xrange(residual_size.value) ])

    _mpl_lib.free_matches(ct.byref(matches_arr))
    _mpl_lib.free_double(ct.byref(residual_arr))




matches_to_restore = (matches.shape[1] * FilterMatch)()
for mi, m in enumerate(matches_to_restore):
    m.__init__(int(matches[0, mi]), float(matches[1, mi]), int(matches[2, mi]))

# matches_to_restore_ptr = ct.cast(matches_to_restore, ct.POINTER(FilterMatch))

print matches_to_restore[0].Fi, matches_to_restore[0].S, matches_to_restore[0].T

restored_arr = ct.POINTER(ct.c_double)()
restored_size = ct.c_uint(0)

_mpl_lib.restore(
    matches_to_restore, len(matches_to_restore),
    filts, filters_num, filter_size,
    ct.byref(restored_arr), ct.byref(restored_size)    
)
restored = np.asarray([ restored_arr[ri] for ri in xrange(restored_size.value) ])
_mpl_lib.free_double(ct.byref(restored_arr))

plt.figure(1)
plt.subplot(2,1,1)
plt.scatter(matches[2,:], matches[0,:])
plt.subplot(2,1,2)
plt.imshow(filts)
plt.show()

plt.plot(residual[:5000])
plt.plot(data[:5000])
plt.show()

# restored = restored/np.sqrt(np.sum(restored ** 2.0))

plt.plot(restored[0:50000])
plt.plot(data[0:50000])
plt.show()

# lr.output.write_wav(pj(res_dir, "{}_result.wav".format(source_id)), restored, sr)