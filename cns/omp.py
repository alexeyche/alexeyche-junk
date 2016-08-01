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
from time import time
import numpy as np
from scipy import linalg
import matplotlib.pyplot as pl
from scipy import optimize
from mp_lib import generate_dct_dictionary


def unsparse(v, idx, length):
    """Transform a vector-index pair to a dense representation"""
    x = np.zeros(length)
    x[idx] = v
    return x

def cholesky_omp(D, x, m, eps=None):
    if eps == None:
        stopping_condition = lambda: it == m  # len(idx) == m
    else:
        stopping_condition = lambda: np.inner(residual, residual) <= eps

    alpha = np.dot(x, D)
    
    #first step:        
    it = 1
    lam = np.abs(np.dot(x, D)).argmax()
    idx = [lam]
    L = np.ones((1,1))
    gamma = linalg.lstsq(D[:, idx], x)[0]
    residual = x - np.dot(D[:, idx], gamma)
    
    while not stopping_condition():
    	lam = np.abs(np.dot(residual, D)).argmax()
        w = linalg.solve_triangular(L, np.dot(D[:, idx].T, D[:, lam]),
                                    lower=True, unit_diagonal=True)
        # should the diagonal be unit in theory? It crashes without it
        L = np.r_[np.c_[L, np.zeros(len(L))],
                  np.atleast_2d(np.append(w, np.sqrt(1 - np.dot(w.T, w))))]
        idx.append(lam)
        it += 1
        #gamma = linalg.solve(np.dot(L, L.T), alpha[idx], sym_pos=True)
        # what am I, stupid??
        Ltc = linalg.solve_triangular(L, alpha[idx], lower=True)
        gamma = linalg.solve_triangular(L, Ltc, trans=1, lower=True)
        residual = x - np.dot(D[:, idx], gamma)
        print np.inner(residual, residual)
    return gamma, idx


ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

res_dir = pj(os.environ["HOME"], "Music", "ml", "impro_res")
if not os.path.exists(res_dir):
    os.makedirs(res_dir)


L = 250
filters_num = 500
jobs = 1
batch_size = 100000
learning_rate = 0.001
epochs = 1
learning = False
target_sr = 3000
source_id = 1
sparsity = int(0.2 * filters_num)
#D = np.random.randn(L, filters_num)
#D = D/np.sqrt(np.sum(D ** 2, 0))
D = generate_dct_dictionary(filters_num, L).T

filters_init = D.copy()


data, source_sr = lr.load(data_source[source_id])
data = lr.resample(data, source_sr, target_sr, scale=True)


starts = np.arange(1500, 1500+L*100, L)

# data_test = lr.resample(data[starts[0]:(starts[-1]+L)], target_sr, source_sr, scale=True)
# lr.output.write_wav(pj(res_dir, "{}_test_resample.wav".format(source_id)), data_test, source_sr)

Yacc = []
for i in starts:
	Y = data[i:(i+L)]
	Ydenom = np.sqrt(np.sum(Y ** 2))
	Y = Y/Ydenom
	
	x, idx = cholesky_omp(D, Y, sparsity)

	Yr = np.dot(D, unsparse(x, idx, filters_num))
	Yacc.append(Yr*Ydenom)

Yc = np.concatenate(Yacc)

data_recov = lr.resample(Yc, target_sr, source_sr, scale=True)
lr.output.write_wav(pj(res_dir, "{}_test_recovery.wav".format(source_id)), data_recov, source_sr)
