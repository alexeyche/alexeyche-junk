#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from numpy import genfromtxt

from crbm import CRBM

csvfile = '/home/alexeyche/my/git/alexeyche-junk/ml/dbn/th/ts_toy.csv'

data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True) 

n_delay = 3
crbm = CRBM(num_vis = num_dims, num_hid = 15, n_delay = n_delay)

(vb_c, hb_c) = crbm.calc_bias_c()

batch_size = 100

index = T.ivector()

hist = []
for h in xrange(1,n_delay+1):
    hist.append(data_sh[index-h])
history = T.concatenate(hist).reshape((n_delay, index.shape[0], num_vis))

f = theano.function([index],[vb_c, hb_c], givens = [( crbm.input, data_sh[index]), 
                                                    ( crbm.history, history)] )
