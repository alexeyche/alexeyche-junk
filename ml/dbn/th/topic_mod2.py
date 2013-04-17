#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os, sys
from theano.tensor.shared_randomstreams import RandomStreams
from theano import ProfileMode
import cPickle

from rbm_rs import RBMReplSoftmax
from rbm_stack import RBMStack
from rbm_util import gray_plot, print_top_to_file
from ae import AutoEncoder
import db_redis as rd
from ais import *

from rbm_classic import RBM

csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/test_data_rs.csv"
data = np.asarray(genfromtxt(csvfile, delimiter=','), dtype=theano.config.floatX)
data_nop = data

perm = np.random.permutation(data.shape[0])
data = data[perm]

valid_num = 50
data_valid = data[-valid_num:]
data = data[:-valid_num]


num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims



data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
data_valid_sh = theano.shared(np.asarray(data_valid, dtype=theano.config.floatX), borrow=True)

train_params = {  'batch_size' : 42, 
                  'learning_rate' : 0.05, 
                  'cd_steps' : 5, 
                  'max_epoch' : 100, 
                  'persistent_on' : True, 
                  'init_momentum' : 0.5, 
                  'momentum' : 0.9, 
                  'moment_start' : 0.01, 
                  'weight_decay' : 0.0002, 
                  'introspect_freq' : 10,
              } 
num_hid = 60

rbm = RBMReplSoftmax(num_vis = num_vis, num_hid = num_hid, from_cache = True)

from rbm_util import *

steps = 100
numpy_rng = np.random.RandomState(1)
#init_vis = theano.shared(np.asarray(3*np.abs(numpy_rng.randn(10, rbm.num_vis)), dtype=theano.config.floatX))
init_vis = theano.shared(data_nop[-100:])
[pre_sigmoid_h1, h1_mean, h1_sample,
            pre_sigmoid_v1, v1_mean, v1_sample], updates = theano.scan(rbm.gibbs_vhv, outputs_info = [None,None,None,None,None,init_vis], n_steps=steps)
f = theano.function([], v1_mean[-1], givens=[(rbm.input, init_vis)], updates = updates)            
v = f()

#rbms = RBMStack(rbms=[rbm])
#rbms.pretrain(data_sh, train_params)
#rbm.save_model()
