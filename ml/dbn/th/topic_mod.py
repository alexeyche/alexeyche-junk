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

csvfile = '/home/alexeyche/my/dbn/topic_mod/scripts/nips_feats.csv'
#csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/sentiment/training_feat.csv"
data = np.asarray(genfromtxt(csvfile, delimiter=','), dtype=theano.config.floatX)
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims


perm = np.random.permutation(num_cases)
data = data[perm]
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)

train_params = {'batch_size' : 5, 'learning_rate' : 0.01, 'cd_steps' : 2, 'max_epoch' : 50, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }

#rbm_rs = RBMStack(num_vis = 10, hid_layers_size = [5], bottomRBMtype = RBMReplSoftmax) 
data_dum_np = np.asarray([[5,6,7,2,3,4,1,30,0,0],[0,2,5,1,0,0,1,4,5,6],[5,30,7,1,0,1,2,0,1,2],[20,0,1,0,1,3,4,10,4,1],[1,0,1,3,10,5,7,1,5,1]], dtype=theano.config.floatX)
data_dum = theano.shared(data_dum_np, borrow=True)

rbm = RBMReplSoftmax(num_vis = 10, num_hid = 10, from_cache = False)

train_params['persistent'] = None
max_epoch = 50
num_batches = 1 #data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
train_params['ep_inc'] = np.float32(1.0/(num_batches*max_epoch))
ep_inc = train_params['ep_inc']
cost, free_en, gparam, updates = rbm.get_cost_updates(train_params)
updates.update([(rbm.epoch_ratio, rbm.epoch_ratio + ep_inc)])

train_rbm_f = theano.function([], [cost, free_en, gparam],
       updates=updates,
       givens=[(rbm.input, data_dum)])

mean_cost = []
mean_cost_last = 0
for ep in xrange(0, 500):
    for b in xrange(0, 1):
        cost, cur_free_en, cur_gparam = train_rbm_f()
        mean_cost.append(cost)
        epoch_ratio = rbm.epoch_ratio.get_value(borrow=True)
        print "pretrain(%3.1f), layer %s, epoch # %d:%d last mean cost %2.2f (cost: %f) free energy: %f grad: %f" % (epoch_ratio*100,0, ep, b, mean_cost_last, cost, cur_free_en, cur_gparam)
    mean_cost_last = np.mean(mean_cost)
    mean_cost = []
#
#
#steps = 30
#numpy_rng = np.random.RandomState(1)
#init_vis = theano.shared(np.asarray(0.01 * numpy_rng.randn(10, rbm.num_vis), dtype=theano.config.floatX))
#[pre_sigmoid_h1, h1_mean, h1_sample,
#            pre_sigmoid_v1, v1_mean, v1_sample], updates = theano.scan(rbm.gibbs_vhv, outputs_info = [None,None,None,None,None,init_vis], n_steps=steps)
#f = theano.function([], v1_sample[-1], updates = updates, givens = [(rbm.input, init_vis)])            
#v = f()
