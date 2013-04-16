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
from ais import AIS

from rbm_classic import RBM

#csvfile = "/home/alexeyche/prog/topic/nips_feats.csv"
#csvfile = "/home/alexeyche/my/dbn/topic_mod/scripts/nips_feats.csv"
#csvfile = "/home/alexeyche/prog/alexeyche-junk/ml/dbn/test_data_rs.csv"
#csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/test_data_rs.csv"
csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/test_data_rbm.csv"
data = np.asarray(genfromtxt(csvfile, delimiter=','), dtype=theano.config.floatX)
#data = np.round(np.log(data[:,0:2000]+1))
#data = data[:,0:2000]
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

train_params = {'batch_size' : 50, 'learning_rate' : 0.1, 'cd_steps' : 1, 'max_epoch' : 2000, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0002, 'introspect_freq' : 10 } 
num_hid = 15

if len(sys.argv)>1:
    num_hid = int(sys.argv[1])

#rbm = RBMReplSoftmax(num_vis = num_vis, num_hid = num_hid, from_cache = True)
rbm = RBM(num_vis = num_vis, num_hid=num_hid, from_cache=True)
#rbms = RBMStack([rbm])
#rbms.pretrain(data_sh, train_params)
#rbm.save_model()
np.savetxt("/home/alexeyche/tmp/W_vals", rbm.W.get_value(borrow=True), delimiter=",")
np.savetxt("/home/alexeyche/tmp/vis_bias_vals", rbm.vbias.get_value(borrow=True), delimiter=",")
np.savetxt("/home/alexeyche/tmp/hid_bias_vals", rbm.hbias.get_value(borrow=True), delimiter=",")

ais = AIS(rbm) #, data=data)
f = ais.perform()

v_mean = T.nnet.sigmoid(ais.base_vbias) 
v_mean_rep = T.tile(v_mean, (ais.numruns,)).reshape((ais.numruns, ais.model.num_vis))
v = ais.theano_rng.binomial(size=v_mean_rep.shape, n=1, p=v_mean_rep, dtype=theano.config.floatX)
# init logw with beta = 0
logw = - ais.log_p_k(v, 0)

for beta in np.arange(0,1,0.01):
    logw += ais.log_p_k(v, beta)
    h = ais.sample_h_given_v(v, beta)
    v = ais.sample_v_given_h(h, beta)
    logw -= ais.log_p_k(v, beta)

logw += ais.log_p_k(v, 1)

f = theano.function([], logw)

