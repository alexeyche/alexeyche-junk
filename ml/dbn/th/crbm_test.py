#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from numpy import genfromtxt

from rbm_util import gray_plot
from crbm import CRBM
from crbm import daydream

from rpy2.robjects.packages import importr

#csvfile = '/home/alexeyche/my/git/alexeyche-junk/ml/dbn/th/ts_toy.csv'
csvfile = '/home/alexeyche/prog/alexeyche-junk/ml/dbn/th/ts_toy.csv'
data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True) 

n_delay = 3
crbm = CRBM(num_vis = num_dims, num_hid = 15, n_delay = n_delay)

(vb_c, hb_c) = crbm.calc_bias_c()

batch_size = 100
batches = np.asarray(np.random.permutation(num_cases), dtype='int32')

# history calculations:
index = T.ivector()
hist = []
for h in xrange(0,n_delay):
    hist.append(data_sh[index-1-h])   # magic of python: if we get negative index, python would give elements from the end
history = T.stack(hist)[0]  # zero index is consenquence of our list trick for shape stack
get_hist = theano.function([index], history)

train_params = {'learning_rate' : 0.01, 'cd_steps' : 1, 'n_delay' : n_delay , 'batch_size' : batch_size, 'weight_decay' : 0.0002, 
                'momentum' : 0.9, 'init_momentum' : 0, 'moment_start' : 5} 

# get_cost_updates compilation
recon_cost, free_energy, energy_cost, updates = crbm.get_cost_updates(train_params, index)
train = theano.function([index],[recon_cost, free_energy, energy_cost], updates=updates, givens = [( crbm.input, data_sh[index]), (crbm.history, history)] )


grdevices = importr('grDevices')
#daydream(crbm, get_hist([4]), 10)

num_batches = len(batches)/batch_size
max_epoch = 1000
if crbm.need_train:
    for epoch in xrange(1, max_epoch+1):
        for b in xrange(0, num_batches):
            batch = batches[b*batch_size:batch_size+b*batch_size]
            rcost, fe, en_cost = train(batch)
            print "Epoch(%d),Batch(%d) Rec.cost: %.4f Free energy: %.4f Energy cost: %.4f" % (epoch, b, rcost, fe, en_cost)
        #grdevices.png(file="/home/alexeyche/tmp/models/W_gray_plot_e_%s.png" % epoch, width=512, height=512)
        #w = crbm.W.get_value(borrow=True)
        #gray_plot(w.tolist(), min=w.min(), max=w.max())
        #grdevices.dev_off()
crbm.save_model()
test = daydream(crbm, get_hist([4]))
#f=theano.function([index], crbm.prop_up(crbm.input), givens = [( crbm.input, data_sh[index]), (crbm.history, history)])

