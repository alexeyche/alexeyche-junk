#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os, sys, time
from theano.tensor.shared_randomstreams import RandomStreams
from theano import ProfileMode
import cPickle

from rbm_rs import RBMReplSoftmax
from rbm_stack import RBMStack
from rbm_util import gray_plot, print_top_to_file
from ae import AutoEncoder
#import db_redis as rd
from db_redis import load_bin
import db_redis
from ais import *

from rbm_classic import RBM

#csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/test_data_rs.csv"
csvfile = "/home/alexeyche/my/dbn/topic_mod/topictoolbox/nips_feats.csv"
data = np.asarray(genfromtxt(csvfile, delimiter=','), dtype=theano.config.floatX)
data_nop = data

perm = np.random.permutation(data.shape[0])
data = data[perm]

valid_num = 50
data_valid = data[-valid_num:]
data = data[:-valid_num]

#data = data[:50]


num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims



data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
data_valid_sh = theano.shared(np.asarray(data_valid, dtype=theano.config.floatX), borrow=True)
data_nop_sh = theano.shared(np.asarray(data_nop, dtype=theano.config.floatX), borrow=True)

train_params = {  'batch_size'             : 50, 
                  'learning_rate'          : 1e-05,
                  'cd_steps'               : 1, 
                  'max_epoch'              : 200, 
                  'persistent_on'          : False, 
                  'init_momentum'          : 0, 
                  'momentum'               : 0, 
                  'moment_start'           : 0.01, 
                  'weight_decay'           : 0.0002, 
                  'mean_field'             : True,
                  'introspect_freq'        : 10,
                  'sparse_cost'            : 0.01,
                  'sparse_damping'         : 0.9,
                  'sparse_target'          : 0.2,
                  'learning_rate_line'     : 0.001, 
                  'finetune_learning_rate' : 0.0001,
              } 
num_hid = 500

rbm = RBMReplSoftmax(num_vis = num_vis, num_hid = num_hid, train_params = train_params, from_cache = False)


#preh, h = rbm.prop_up(data_sh[0:100])
#f = theano.function([], [preh,h], givens=[(rbm.input, data_sh[0:100])] )

db_redis.r0.flushdb()

iter=0
def load_watches(watches):
    global iter
    start = time.time()   
    for k in watches:
        name = "%i:%s" % (iter, k)
        load_bin(name, watches[k])
    db_redis.r0.set("last_it", iter)
    iter+=1
    end = time.time()
    print "Inserted in %s" % (end - start,)

rbms = RBMStack(rbms=[rbm])
for watches in rbms.pretrain(data_sh, data_valid_sh, train_params):
    load_watches(watches)
    
#train_params['max_epoch'] = 200
#train_params['mean_field'] = False
#train_params['cd_steps'] = 5
#train_params['learning_rate'] = 0.0005
#rbms.stack[0].need_train = True
#for watches in rbms.pretrain(data_sh, data_valid_sh, train_params):
#    print watches

#ae = AutoEncoder(rbms)
#train_params['max_epoch'] = 400
#ae.pretrain(data_sh, data_valid_sh, train_params)
#train_params['max_epoch'] = 50
#ae.finetune(data_sh, train_params)
#print_top_to_file(ae, train_params, "_sp0.2", data_nop_sh, range(0,100)) 
#ais = AIS_RS(rbm, betas = np.asarray(np.concatenate([np.arange(0,0.5,1e-03), np.arange(0.5,0.9,1e-03),np.arange(0.9,1,1e-03)]), dtype=theano.config.floatX), data = data, mean_field=False)
#v_samples, logw_list, vs, logz = ais.est_log_part_fun()
#log_z_est  = ais.est_log_part_fun()
#log_probs = log_prob_data(rbm, log_z_est, data_sh)
#v = data_sh[0:2]
#D = T.sum(v, axis=1)
#h = ais.sample_h_given_v(v, 0.5, D)
#v = ais.sample_v_given_h(h, 0.5, D, 0)
#f = theano.function([],[v,D,h])
