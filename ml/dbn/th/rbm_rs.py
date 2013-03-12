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

from rbm import RBMReplSoftmax, train_rbm
from rbm_stack import RBMStack

csvfile = '/home/alexeyche/my/git/alexeyche-junk/ml/dbn/sentiment/training_feat.csv'

data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
data_target = data[...,0]
data_target_0 = data_target
data = data[...,1:ncol]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

data_wo_p = data[:7000]
data_target_wo_p = data_target

perm = np.random.permutation(num_cases)
data = data[perm]
data_target = data_target[perm]
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)

#rbm_rs = RBMStack(num_vis = num_vis, hid_layers_size = [1000,500], repl_softmax = True)

train_params = {'batch_size' : 100, 'learning_rate' : 0.1, 'cd_steps' : 2, 'max_epoch' : 20, 'persistent' : True }
