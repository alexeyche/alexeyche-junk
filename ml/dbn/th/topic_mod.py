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

csvfile = "/home/alexeyche/prog/topic/nips_feats.csv"
#csvfile = "/home/alexeyche/my/dbn/topic_mod/scripts/nips_feats.csv"
#csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/sentiment/training_feat.csv"
data = np.asarray(genfromtxt(csvfile, delimiter=','), dtype=theano.config.floatX)
data = np.round(np.log(data[:,0:2000]+1))
#data = data[:,0:2000]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

perm = np.random.permutation(num_cases)
data = data[perm]
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)

train_params = {'batch_size' : 50, 'learning_rate' : 0.01, 'cd_steps' : 1, 'max_epoch' : 1000, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }

rbm = RBMReplSoftmax(num_vis = num_vis, num_hid = 200, from_cache = False)

num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
max_epoch = train_params['max_epoch']
train_params['ep_inc'] = np.float32(1.0/(num_batches*max_epoch))
ep_inc = train_params['ep_inc']
persistent = train_params['persistent_on']
batch_size = train_params['batch_size']
index = T.lscalar('index')  # index to a minibatch

if persistent:
    train_params['persistent'] = theano.shared(np.zeros((batch_size, rbm.num_hid), dtype=theano.config.floatX), borrow=True)
else:
    train_params['persistent'] = None
cost, free_en, gparam, updates = rbm.get_cost_updates(train_params)
updates.update([(rbm.epoch_ratio, rbm.epoch_ratio + ep_inc)])

f = theano.function([index], [cost, free_en, gparam],
       updates=updates,
       givens=[(rbm.input, data_sh[index * batch_size: (index + 1) * batch_size])])

get_watches = theano.function([index], rbm.watches, updates=updates,givens=[(rbm.input, data_sh[index * batch_size: (index + 1) * batch_size])])

it=0

def print_watches(w):
    global it
    for k, v in w:
        if len(v.shape) == 1:
            print "%s:\n%s" % (k, v[0:10])
        if len(v.shape) == 2:
            if v.shape[0] > 200:
                v = v[0:200]
            if v.shape[1] > 200:
                v = v[:, 0:200]
            print "%s:\n%s" % (k, v[0:3,0:10])
        rd.load("%d:%s" % (it, k), v)
        rd.r0.set("last_it", it)
    it+=1
#    import pdb; pdb.set_trace()

def train_rs(rbm, train_params):
    max_epoch = train_params['max_epoch']
    num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']

    mean_cost = []
    mean_cost_last = 0
    for ep in xrange(0, max_epoch):
        for b in xrange(0, num_batches):
            cost, cur_free_en, cur_gparam = f(b)            
            w = zip(rbm.watches_label, get_watches(b)) # map to dict from labels-values lists
            mean_cost.append(cost)
            epoch_ratio = rbm.epoch_ratio.get_value(borrow=True)
            print "pretrain(%3.1f), layer %s, epoch # %d:%d last mean cost %2.2f (cost: %f) free energy: %f grad: %f" % (epoch_ratio*100,0, ep, b, mean_cost_last, cost, cur_free_en, cur_gparam)
        mean_cost_last = np.mean(mean_cost)
        mean_cost = []
        if ep % 10 == 0:
            print_watches(w)

if rbm.need_train:
    rd.r0.flushdb()
    train_params = {'batch_size' : 87, 'learning_rate' : 0.1, 'cd_steps' : 1, 'max_epoch' : 300, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }

    train_rs(rbm, train_params)

#    train_params = {'batch_size' : 87, 'learning_rate' : 0.01, 'cd_steps' : 3, 'max_epoch' : 300, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }
#
#    train_rs(rbm, train_params)
#
#    train_params = {'batch_size' : 87, 'learning_rate' : 0.01, 'cd_steps' : 5, 'max_epoch' : 400, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }
#
#    train_rs(rbm, train_params)
#    rbm.save_model()

#rbm_st = RBMStack(rbms = [rbm])
#ae = AutoEncoder(rbm_st)
#train_params['learning_rate_line'] = 0.001
#train_params['max_epoch'] = 50
#ae.pretrain(data_sh, train_params)
#train_params['finetune_learning_rate'] = 0.1
#train_params['max_epoch'] = 100
#ae.finetune(data_sh, train_params)
#print_top_to_file(ae, train_params, "rs", data_sh, range(0,1000))
