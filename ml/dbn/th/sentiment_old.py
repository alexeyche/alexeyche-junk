#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
from theano import ProfileMode
import cPickle
###################################
from rbm_util import daydream
from rbm import RBM, RBMBinLine, train_rbm
from fine_tune import get_error, fine_tune

csvfile = '/home/alexeyche/my/dbn/kaggle_sentiment/training_feat_proc.csv'
#csvfile = '/home/alexeyche/prog/alexeyche-junk/ml/dbn/sentiment/training_feat_proc.csv'
data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
data_target = data[...,0]
data = data[...,1:ncol]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

perm = np.random.permutation(num_cases)
data = data[perm]
data_target = data_target[perm]
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
data_target_sh = theano.shared(np.asarray(data_target, dtype=theano.config.floatX), borrow=True)



rbm = RBM(num_vis = num_dims, num_hid = 500)
rbm_line = RBMBinLine(num_vis = 500, num_hid = 2)

# hyper parameters
train_params = { 'batch_size' : 100, 'learning_rate' : 0.1, 'cd_steps' : 2, 'max_epoch' : 25, 'persistent' : False}

train_rbm(rbm, data_sh, train_params, False, False)
fine_tune(rbm, data_sh, epochs = 10, batch_size=100)

# collect statistics
pre_sigm, hid_stat = theano.function([], rbm.prop_up(data_sh))()
hid_stat_sh = theano.shared(np.asarray(hid_stat, dtype=theano.config.floatX), borrow=True)

# hyper parameters
train_params = { 'batch_size' : 100, 'learning_rate' : 0.05, 'cd_steps' : 1, 'max_epoch' : 20, 'persistent' : False }
train_rbm(rbm_line, hid_stat_sh, train_params, False, False)

hid_stat = theano.function([], rbm.prop_up(hid_stat_sh))()
x = hid_stat[0:1000,0].tolist()
y = hid_stat[0:1000,1].tolist()
import rpy2.robjects as ro
lab = ro.IntVector(data_target[0:100].tolist())
lab_col = ro.StrVector(map(lambda p: p == 0 and 'blue' or 'red', lab))
lab_col.names = lab
ro.r.plot(x,y, xlab = "x", ylab="y", type="n")
ro.r.text(x,y,labels=lab, col = lab_col)
