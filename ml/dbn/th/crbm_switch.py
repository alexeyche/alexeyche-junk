#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from numpy import genfromtxt

from rbm_util import gray_plot
from crbm import CRBMSoftmax
from crbm import generate_case

from rpy2.robjects.packages import importr


csvfile = "/home/alexeyche/my/dbn/code_patterns.less2.csv"
#csvfile = "/home/alexeyche/my/dbn/test_patterns.csv"

data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims
num_cases_fake = 1000000

data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True) 

n_delay = 2
num_hid = 100
crbm = CRBMSoftmax(num_vis = num_dims, num_hid = num_hid, n_delay = n_delay)

batch_size = 100

batches = np.asarray(np.random.permutation(num_cases), dtype='int32')
batches = batches[:num_cases_fake]
# history calculations:
index = T.ivector()

stop_case = np.zeros((1,ncol), dtype=theano.config.floatX)
stop_case[0,ncol-1] = 1

def filter_stop_case(d, filter_vector = None):
    eq = T.eq(d, stop_case)
    f = T.prod(eq, axis=1)
    if filter_vector:
        f = T.or_(filter_vector, f) # increment new overlapped history
    f_inv = T.abs_(f-1)
    d_f = d.T * f_inv.T
    d_f = T.cast(d_f.T, 'float32')
    return d_f, f

hist = []
f_v = []
filter_vector=None
for h in xrange(0,n_delay):
    hist_f, filter_vector = filter_stop_case(data_sh[index-1-h], filter_vector)   # history is not overlapped
    hist.append(hist_f)
#    f_v.append(filter_vector)
history = T.stack(hist)[0]  # zero index is consenquence of our list trick for shape stack
#f_v_m = T.stack(f_v)[0]
get_hist = theano.function([index], history)



max_epoch = 5
num_batches = len(batches)/batch_size
ep_inc = np.float32(1.0/(num_batches*max_epoch))

train_params = {'learning_rate' : 0.1, 'cd_steps' : 3, 'n_delay' : n_delay , 'batch_size' : batch_size, 'weight_decay' : 0.0002, 
                'momentum' : 0.9, 'init_momentum' : 0, 'moment_start' : 0.01 } 


recon_cost, free_energy, energy_cost, updates = crbm.get_cost_updates(train_params)
updates.update([(crbm.epoch_ratio, crbm.epoch_ratio + ep_inc)])
train = theano.function([index],[recon_cost, free_energy, energy_cost], updates=updates, givens = [(crbm.input, data_sh[index]), (crbm.history, history)] )
if crbm.need_train:
    for epoch in xrange(1, max_epoch+1):
        for b in xrange(0, num_batches):
            batch = batches[b*batch_size:batch_size+b*batch_size]
            rcost, fe, en_cost = train(batch)
            print "Epoch(%d),Batch(%d) Rec.cost: %.4f Free energy: %.4f Energy cost: %.4f" % (epoch, b, rcost, fe, en_cost)
    crbm.save_model()            

#preh, h  = crbm.prop_up(crbm.input)

#pre_sigm, v_mean, v_act = crbm.sample_v_given_h(h)
#f = theano.function([index],[v_mean, v_act], givens = [(crbm.input, data_sh[index]), (crbm.history, history)] )

s = generate_case(crbm)

