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
                  'max_epoch' : 200, 
                  'persistent_on' : True, 
                  'init_momentum' : 0.5, 
                  'momentum' : 0.9, 
                  'moment_start' : 0.01, 
                  'weight_decay' : 0.0002, 
                  'mean_field' : True,
                  'introspect_freq' : 10,
              } 
num_hid = 60

rbm = RBMReplSoftmax(num_vis = num_vis, num_hid = num_hid, from_cache = False)

#from rbm_util import *
#
#steps = 100
#numpy_rng = np.random.RandomState(1)
#init_vis = theano.shared(data_nop[0:5])
#preh, h, hs = rbm.sample_h_given_v(init_vis)
#prev, v, vs = rbm.sample_v_given_h(hs, 1)
#f = theano.function([], vs, givens=[(rbm.input, init_vis)])
#v = f()

rbms = RBMStack(rbms=[rbm])
rbms.pretrain(data_sh, train_params)
rbm.save_model()

#ais = AIS_RS(rbm, betas = np.asarray(np.concatenate([np.arange(0,0.5,1e-03), np.arange(0.5,0.9,1e-03),np.arange(0.9,1,1e-03)]), dtype=theano.config.floatX), data = data, mean_field=False)
#v_samples, logw_list, vs, logz = ais.est_log_part_fun()
#log_z_est  = ais.est_log_part_fun()
#log_probs = log_prob_data(rbm, log_z_est, data_sh)
#v = data_sh[0:2]
#D = T.sum(v, axis=1)
#h = ais.sample_h_given_v(v, 0.5, D)
#v = ais.sample_v_given_h(h, 0.5, D, 0)
#f = theano.function([],[v,D,h])
