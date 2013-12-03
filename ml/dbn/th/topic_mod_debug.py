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

train_params = {'batch_size' : 5, 'learning_rate' : 0.01, 'cd_steps' : 2, 'max_epoch' : 20, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }

#rbm_rs = RBMStack(num_vis = 10, hid_layers_size = [5], bottomRBMtype = RBMReplSoftmax) 
data_dum_np = np.asarray([[5,6,7,2,3,4,1,30,0,0],[0,2,5,1,0,0,1,4,5,6],[5,30,7,1,0,1,2,0,1,2],[20,0,1,0,1,3,4,10,4,1],[1,0,1,3,10,5,7,1,5,1]], dtype=theano.config.floatX)

#data_dum_np = np.round(np.log(data_dum_np+1))
data_dum = theano.shared(data_dum_np, borrow=True)

rbm = RBMReplSoftmax(num_vis = 10, num_hid = 10, from_cache = False)
#preh, hm, hs = rbm.sample_h_given_v(rbm.input)
#prev, vm, vs = rbm.sample_v_given_h(hs)
#
#f = theano.function([], [preh,hm,hs,prev,vm,vs], givens = [(rbm.input, data_sh[0:3])])

index = T.lscalar('index')  # index to a minibatch

max_epoch = train_params['max_epoch']
#num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']

#train_params['ep_inc'] = np.float32(1.0/(num_batches*max_epoch))

#ep_inc = train_params['ep_inc']
l_rate = T.cast(train_params['learning_rate'], dtype=theano.config.floatX)
weight_decay = T.cast(train_params['weight_decay'], dtype=theano.config.floatX)
momentum = T.cast(train_params['momentum'], dtype=theano.config.floatX)
init_momentum = T.cast(train_params['init_momentum'], dtype=theano.config.floatX)
moment_start = train_params['moment_start']

batch_size = T.cast(train_params['batch_size'], dtype=theano.config.floatX)
cd_steps = train_params['cd_steps']

# compute positive phase
pre_sigmoid_ph, ph_mean, ph_sample = rbm.sample_h_given_v(rbm.input)

pre_softmax_v, v_mean = rbm.prop_down(ph_sample)
v_sample = rbm.theano_rng.multinomial(n=rbm.D, pvals=v_mean, dtype=theano.config.floatX)
#rbm.D = T.sum(v_sample, axis=1)
pre_sigmoid_h, h_mean = rbm.prop_up(v_sample)
h_sample = rbm.theano_rng.binomial(size=h_mean.shape, n=1, p=h_mean, dtype=theano.config.floatX)

pre_softmax_v, v_mean = rbm.prop_down(h_sample)
v_sample = rbm.theano_rng.multinomial(n=rbm.D, pvals=v_mean, dtype=theano.config.floatX)
#rbm.D = T.sum(v_sample, axis=1)
pre_sigmoid_h, h_mean = rbm.prop_up(v_sample)
h_sample = rbm.theano_rng.binomial(size=h_mean.shape, n=1, p=h_mean, dtype=theano.config.floatX)

pre_softmax_v, v_mean = rbm.prop_down(h_sample)
v_sample = rbm.theano_rng.multinomial(n=rbm.D, pvals=v_mean, dtype=theano.config.floatX)
#rbm.D = T.sum(v_sample, axis=1)
pre_sigmoid_h, h_mean = rbm.prop_up(v_sample)
h_sample = rbm.theano_rng.binomial(size=h_mean.shape, n=1, p=h_mean, dtype=theano.config.floatX)


vis_probs_fant = v_sample #[-1]
hid_probs_fant = h_mean #[-1]

W_inc = ( T.dot(rbm.input.T, ph_mean) - T.dot(vis_probs_fant.T, hid_probs_fant) )/batch_size  - rbm.W * weight_decay
hbias_inc = (T.sum(ph_mean, axis=0) - T.sum(hid_probs_fant,axis=0))/batch_size
vbias_inc = (T.sum(rbm.input,axis=0) - T.sum(vis_probs_fant,axis=0))/batch_size

cur_momentum = 0.9

W_inc_rate = (rbm.W_inc * cur_momentum + W_inc) * l_rate
hbias_inc_rate = (rbm.hbias_inc * cur_momentum + hbias_inc) * l_rate
vbias_inc_rate = (rbm.vbias_inc * cur_momentum + vbias_inc) * l_rate
updates=[]
updates.append( (rbm.W, rbm.W + W_inc_rate))
updates.append((rbm.hbias,rbm.hbias + hbias_inc_rate))
updates.append((rbm.vbias,rbm.vbias + vbias_inc_rate))
updates.append((rbm.W_inc,W_inc))
updates.append((rbm.hbias_inc,hbias_inc))
updates.append((rbm.vbias_inc,vbias_inc))
current_free_energy = rbm.free_energy(rbm.input)
        
monitoring_cost = rbm.get_reconstruction_cost(v_sample)

f = theano.function([], [ph_sample, v_mean, h_mean, vis_probs_fant, hid_probs_fant, W_inc_rate, vbias_inc_rate, hbias_inc_rate, current_free_energy, monitoring_cost], updates=updates, givens=[(rbm.input, data_dum)])

sumcost = []
def it(i):
    global sumcost
    ph_sample, vis_probs_fant, hid_probs_fant, vis_samp, hid_samp, W_inc, vbias_inc, hbias_inc, fen, cost = f()
    if i % 10 == 0:
        print "hid_pos: %s" % ph_sample
        print "vis_neg: %s" % vis_probs_fant
        print "hid_neg: %s" % hid_probs_fant
        print "vis_neg_s: %s" % vis_samp
        print "hid_neg_s: %s" % hid_samp
        print "W_inc: %s" % W_inc
        print "vbias_inc: %s" % vbias_inc
        print "hbias_inc: %s" % hbias_inc
        print "free en: %s" % fen
        print "cost: %s" % np.mean(sumcost)
        sumcost = []
    else:
        sumcost.append(cost)

import time

for i in xrange(1,1000):
    time.sleep(0.1)
    it(i)


#cost, free_en, gparam, updates = rbm.get_cost_updates(train_params)
#updates.update([(rbm.epoch_ratio, rbm.epoch_ratio + ep_inc)])
#
#train_rbm_f = theano.function([], [cost, free_en, gparam],
#       updates=updates,
#       givens=[(rbm.input, data_dum)])
#
#for ep in xrange(0, 50):
#    for b in xrange(0, 1):
#        cost, cur_free_en, cur_gparam = train_rbm_f()
#        epoch_ratio = rbm.epoch_ratio.get_value(borrow=True)
#        print "pretrain(%3.1f), layer %s, epoch # %d:%d cost: %f free energy: %f grad: %f" % (epoch_ratio*100,0, ep, b, cost, cur_free_en, cur_gparam)
#
#
#steps = 30
#numpy_rng = np.random.RandomState(1)
#init_vis = theano.shared(np.asarray(0.01 * numpy_rng.randn(10, rbm.num_vis), dtype=theano.config.floatX))
#[pre_sigmoid_h1, h1_mean, h1_sample,
#            pre_sigmoid_v1, v1_mean, v1_sample], updates = theano.scan(rbm.gibbs_vhv, outputs_info = [None,None,None,None,None,init_vis], n_steps=steps)
#f = theano.function([], v1_sample[-1], updates = updates, givens = [(rbm.input, init_vis)])            
#v = f()
