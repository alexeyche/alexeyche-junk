#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from numpy import genfromtxt

from rbm_stack import RBMStack
from rbm_mult_softmax import RBMMultSoftmax



csvfile = "/home/alexeyche/my/dbn/code_patterns.less2.csv"
#csvfile = "/home/alexeyche/my/dbn/test_patterns.csv"

data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims
num_cases_fake = 1000000


num_units = 20 # length of case
unit_size = 9

def preprocess(num_cases = 1000):
    stop_case = np.zeros((num_dims), dtype=theano.config.floatX)
    stop_case[num_dims-1] = 1
    
    data_train = np.zeros((num_cases, num_units*unit_size), dtype=theano.config.floatX)
    hist=[]
    i = 0
    j = 0
    for case in data:
        j+=1
        if (case - stop_case).any() == 0:
            if len(hist) > num_units:
                print "We missed long case(%s): %s" % (len(hist), i)
                del hist[:]
                continue
            case_arr = np.concatenate(hist, axis=1)
            l = case_arr.shape[0]
            data_train[i,0:l] = case_arr
            i+=1
            del hist[:]
            if i == num_cases:
                break
        else:
            hist.append(case[:-1])
    return data_train

data_train = preprocess()
data_train_sh = theano.shared(np.asarray(data_train, dtype=theano.config.floatX), borrow=True) 

rbms = RBMStack(hid_layers_size = [15], bottomRBMtype = RBMMultSoftmax, add_opts = { 'num_units' : num_units, 'unit_size' : unit_size })

train_params = { 'max_epoch' : 50, 'batch_size' : 50, 'learning_rate' : 0.001, 'cd_steps' : 15, 'persistent' : True} 
rbms.pretrain(data_train_sh, train_params)

rbm = rbms.stack[0]
rbm.save_model()

def evaluate(data):
    def softmax(w):
        e = np.exp(w)
        dist = e / np.sum(e)
        return dist

    w = rbm.W.get_value(borrow=True)
    vbias = rbm.vbias.get_value(borrow=True)
    hbias = rbm.hbias.get_value(borrow=True)
     
    for batch_n in xrange(0,1): #, data.shape[0]/50): 
        case = data[batch_n*50:batch_n*50+50]
        for i in xrange(0, unit_size):
            #case_wo_i = np.concatenate((case[:,:i], case[:,i+1:]), axis=1)
            #w_cust = np.concatenate((w[:i], w[i+1:]), axis=0) 
            case_wo_i = case
            case_wo_i[:,i] = 0
            w_cust = w
            hid_act = np.dot(case_wo_i, w_cust) + hbias 
            pre_vis_act = np.dot(hid_act, w.T) + vbias
            v = []
            for j in xrange(0, num_units):
                v.append(softmax(pre_vis_act[:,j*unit_size:j*unit_size+unit_size]))
            vis = np.concatenate(v, axis=1)
            print np.sum(vis-case)

numpy_rng = np.random.RandomState(1)
init_vis = theano.shared(np.asarray(0.01 * numpy_rng.randn(10, rbm.num_vis), dtype=theano.config.floatX))
[pre_sigmoid_h1, h1_mean, h1_sample,
         pre_sigmoid_v1, v1_mean, v1_sample], updates = theano.scan(rbm.gibbs_vhv, outputs_info = [None,None,None,None,None,init_vis], n_steps=5)
f = theano.function([], v1_mean[-1], updates = updates)            
v = f()
