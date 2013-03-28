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

#csvfile = '/home/alexeyche/my/dbn/topic_mod/scripts/nips_feats.csv'
csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/sentiment/training_feat.csv"
data = np.asarray(genfromtxt(csvfile, delimiter=','), dtype=theano.config.floatX)
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims


perm = np.random.permutation(num_cases)
data = data[perm]
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)

train_params = {'batch_size' : 2, 'learning_rate' : 0.005, 'cd_steps' : 1, 'max_epoch' : 20, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }

rbm_rs = RBMStack(num_vis = 10, hid_layers_size = [5], bottomRBMtype = RBMReplSoftmax) 

data_dum = theano.shared(np.asarray([[5,6,7,2,3,4,1,0,0,0],[0,2,5,1,0,0,1,4,5,6]], dtype=theano.config.floatX), borrow=True)
rbm_rs.pretrain(data_dum, train_params)

#rbm = rbm_rs.stack[0]

#preh, h = rbm.prop_up(data_dum)
#prev, v_m, v_s = rbm.sample_v_given_h(h)
#h_dum = theano.shared(np.asarray([1,2,3], dtype=theano.config.floatX))
#D_dum = theano.shared(np.asarray([5,10,20,30,40], dtype=theano.config.floatX))
#hb = T.outer(D_dum, h_dum)
#f = theano.function([], hb)
