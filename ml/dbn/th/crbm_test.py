#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from numpy import genfromtxt

from rbm_util import gray_plot
from crbm import CRBM
from crbm import generate

from rpy2.robjects.packages import importr

csvfile = '/home/alexeyche/my/git/alexeyche-junk/ml/dbn/th/ts_toy.csv'
#csvfile = '/home/alexeyche/prog/alexeyche-junk/ml/dbn/th/ts_toy.csv'
data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True) 

n_delay = 3
crbm = CRBM(num_vis = num_dims, num_hid = 15, n_delay = n_delay)

batch_size = 100
batches = np.asarray(np.random.permutation(num_cases), dtype='int32')

# history calculations:
index = T.ivector()
hist = []
for h in xrange(0,n_delay):
    hist.append(data_sh[index-1-h])   # magic of python: if we get negative index, python would give elements from the end
history = T.stack(hist)[0]  # zero index is consenquence of our list trick for shape stack
get_hist = theano.function([index], history)

max_epoch = 1000
num_batches = len(batches)/batch_size
ep_inc = np.float32(1.0/(num_batches*max_epoch))

train_params = {'learning_rate' : 0.01, 'cd_steps' : 5, 'n_delay' : n_delay , 'batch_size' : batch_size, 'weight_decay' : 0.0002, 
                'momentum' : 0.9, 'init_momentum' : 0, 'moment_start' : 0.01 } 


recon_cost, free_energy, energy_cost, updates = crbm.get_cost_updates(train_params)
updates.update([(crbm.epoch_ratio, crbm.epoch_ratio + ep_inc)])
train = theano.function([index],[recon_cost, free_energy, energy_cost], updates=updates, givens = [(crbm.input, data_sh[index]), (crbm.history, history)] )


grdevices = importr('grDevices')

plot_weight_epoch = 60
if crbm.need_train:
    for epoch in xrange(1, max_epoch+1):
        for b in xrange(0, num_batches):
            batch = batches[b*batch_size:batch_size+b*batch_size]
            rcost, fe, en_cost = train(batch)
            print "Epoch(%d),Batch(%d) Rec.cost: %.4f Free energy: %.4f Energy cost: %.4f" % (epoch, b, rcost, fe, en_cost)
        if epoch % plot_weight_epoch == 0:
            grdevices.png(file="/home/alexeyche/tmp/models/W_gray_plot_e_%s.png" % epoch, width=512, height=512)
            w = crbm.W.get_value(borrow=True)
            gray_plot(w.tolist(), min=w.min(), max=w.max())
            grdevices.dev_off()

    crbm.save_model()
start_index = 3
samples, samples_prob = generate(crbm, get_hist([start_index]), 1000, 5)

sum_err = 0
sum_err_prob = 0
for i in xrange(0, len(samples)):
    diff = data[start_index+i] - samples[i]
    diff_prob = data[start_index+i] - samples_prob[i]
    sum_err += np.sum(diff)
    sum_err_prob += np.sum(diff_prob)

print "Sample sum error: %s , Prob sum error: %s" % (sum_err, sum_err_prob)    
