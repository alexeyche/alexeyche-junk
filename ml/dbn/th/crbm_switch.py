#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from numpy import genfromtxt

from rbm_util import gray_plot
from crbm import CRBMSoftmax
from crbm import generate

from rpy2.robjects.packages import importr


csvfile = "/home/alexeyche/my/dbn/code_patterns.less.csv"

data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

stop_case = np.zeros((1,ncol))

data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True) 

n_delay = 10
crbm = CRBMSoftmax(num_vis = num_dims, num_hid = 100, n_delay = n_delay)

batch_size = 100
batches = np.asarray(np.random.permutation(num_cases), dtype='int32')

# history calculations:
index = T.ivector()
hist = []
for h in xrange(0,n_delay):
    hist.append(data_sh[index-1-h])   # magic of python: if we get negative index, python would give elements from the end
history = T.stack(hist)[0]  # zero index is consenquence of our list trick for shape stack

get_hist = theano.function([index], history)

d = data_sh[index-1]

eq = T.eq(d, stop_case)
eq2 = T.prod(eq, axis=1)
eq3 = T.abs_(eq2-1)
d_final = d * eq3
f = theano.function([index], [d_final])

quit()




max_epoch = 1000
num_batches = len(batches)/batch_size
ep_inc = np.float32(1.0/(num_batches*max_epoch))

train_params = {'learning_rate' : 0.01, 'cd_steps' : 5, 'n_delay' : n_delay , 'batch_size' : batch_size, 'weight_decay' : 0.0002, 
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



