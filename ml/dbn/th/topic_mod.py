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

#csvfile = "/home/alexeyche/prog/topic/nips_feats.csv"
#csvfile = "/home/alexeyche/my/dbn/topic_mod/scripts/nips_feats.csv"
#csvfile = "/home/alexeyche/prog/alexeyche-junk/ml/dbn/test_data_rs.csv"
csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/test_data_rs.csv"
data = np.asarray(genfromtxt(csvfile, delimiter=','), dtype=theano.config.floatX)
#data = np.round(np.log(data[:,0:2000]+1))
#data = data[:,0:2000]
data_nop = data

perm = np.random.permutation(data.shape[0])
data = data[perm]

valid_num = 25
data_valid = data[-valid_num:]
data = data[:-valid_num]


num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims



data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
data_valid_sh = theano.shared(np.asarray(data_valid, dtype=theano.config.floatX), borrow=True)

train_params = {'batch_size' : 25, 'learning_rate' : 0.005, 'cd_steps' : 1, 'max_epoch' : 151, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0002, 'introspect_freq' : 10 } 

rbm = RBMReplSoftmax(num_vis = num_vis, num_hid = 75, from_cache = False)

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

valid_set = T.matrix("valid_set")
rbm.add_watch(T.mean(rbm.free_energy(valid_set)), "free_en_valid")
Dv = T.sum(valid_set, axis=1)
[preh, h, hs, prev, v, vs], updates = \
            theano.scan(rbm.gibbs_vhv,
                    outputs_info=[None,  None,  None, None, None, valid_set],
                    non_sequences = Dv,
                    n_steps=1)
#rbm.add_watch(rbm.get_reconstruction_cost(vs, valid_set, Dv), "cost_valid")
get_watches = theano.function([index], rbm.watches, updates=updates,givens=[(rbm.input, data_sh[index * batch_size: (index + 1) * batch_size]),
                                                                            (valid_set, data_valid_sh)])

it=0

def load_watches(w):
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


def train_rs(rbm, train_params):
    max_epoch = train_params['max_epoch']
    num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
    introspect_freq = train_params['introspect_freq']

    mean_cost_acc = []
    mean_cost_valid_acc = []
    free_en_acc = []
    free_en_valid_acc = []

    free_en = []
    free_en_last = 0
    mean_cost = []
    mean_cost_last = 0
    free_en_valid = []
    free_en_valid_last = 0    
    mean_cost_valid = []
    mean_cost_valid_last = 0

    intro_b = 0
    for ep in xrange(0, max_epoch):
        for b in xrange(0, num_batches):
            cost, cur_free_en, cur_gparam = f(b)            
            mean_cost.append(cost)
            free_en.append(cur_free_en)

            w = zip(rbm.watches_label, get_watches(b)) # map to list of tuples from labels-values lists
            free_en_v_ind = rbm.watches_label.index("free_en_valid")
            free_en_valid.append(w[free_en_v_ind][1]) 
            #mean_cost_v_ind = rbm.watches_label.index("cost_valid")
            #mean_cost_valid.append(w[mean_cost_v_ind][1])

            epoch_ratio = rbm.epoch_ratio.get_value(borrow=True)
            print "pretrain(%3.1f), layer %s, epoch # %d:%d last mean cost %2.2f (cost: %f) free energy: %f grad: %f" % (epoch_ratio*100,0, ep, b, mean_cost_last, cost, cur_free_en, cur_gparam)
        mean_cost_last = np.mean(mean_cost)
        free_en_last = np.mean(free_en)
        free_en_valid_last = np.mean(free_en_valid)
        mean_cost_valid_last = np.mean(mean_cost_valid)
        mean_cost = []
        free_en = []
        free_en_valid = []
        mean_cost_valid = []
        mean_cost_acc.append(mean_cost_last)
        free_en_acc.append(free_en_last)
        free_en_valid_acc.append(free_en_valid_last)
        mean_cost_valid_acc.append(mean_cost_valid_last)
        if ep % introspect_freq == 0:
            w = zip(rbm.watches_label, get_watches(b-intro_b)) # map to list of tuples from labels-values lists
            intro_b += 1
            if intro_b > 10:
                intro_b = 0
            w.append( ("cost_acc", np.asarray(mean_cost_acc)) )
            w.append( ("free_en_valid_delta", np.asarray(free_en_acc) - np.asarray(free_en_valid_acc)) )
            w.append( ("free_en_acc", np.asarray(free_en_acc)) )
            w.append( ("free_en_valid_acc", np.asarray(free_en_valid_acc)) )
            w.append( ("max_epoch", np.asarray(max_epoch)) )
            w.append( ("cost_valid_acc", np.asarray(mean_cost_valid_acc)) ) 
            w.append( ("cost_delta", np.asarray(mean_cost_valid_acc) - np.asarray(mean_cost_acc)) )
            load_watches(w)

if rbm.need_train:
    rd.r0.flushdb()
    
    train_rs(rbm, train_params)
    rbm.save_model()
#data_nop_sh = theano.shared(data_nop, borrow=True)
#preh, h = rbm.prop_up(data_nop_sh[-100:])
#prev, v, vs = rbm.sample_v_given_h(hs)
#
#test = theano.function([], h, givens = [(rbm.input, data_nop_sh[-100:])])
#before = test()
#train_params['max_epoch'] = 30
#train_params['cd_steps'] = 5
#
#train_rs(rbm, train_params)
#after = test()
#rbm_st = RBMStack(rbms = [rbm])
#ae = AutoEncoder(rbm_st)
#train_params['learning_rate_line'] = 0.1
#train_params['max_epoch'] = 100
#train_params['cd_steps'] = 10
#train_params['persistent_on'] = True
#ae.pretrain(data_sh, train_params)
#train_params['finetune_learning_rate'] = 0.05
#train_params['max_epoch'] = 10
#ae.finetune(data_sh, train_params)
#data_nop_sh = theano.shared(data_nop, borrow=True)
#print_top_to_file(ae, train_params, "rs", data_nop_sh, range(700,1050))
