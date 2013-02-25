#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
from theano import ProfileMode
import cPickle
###################################
from rbm_util import daydream
from rbm import RBM, RBMBinLine, collect_hid_stat
from fine_tune import get_error, fine_tune

csvfile = '/home/alexeyche/my/dbn/kaggle_sentiment/training_feat_proc.csv'
data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
data_target = data[...,0]
data = data[...,1:ncol]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

perm = np.random.permutation(num_cases)
data = data[perm]
data_target = data_target[perm]
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
data_target_sh = theano.shared(np.asarray(data_target, dtype=theano.config.floatX), borrow=True)


# hyper parameters
batch_size = 100
num_hid = 500
lr_giv = 0.05
persistent_giv = None
k_giv = 1
max_epoch = 50 


num_batches = num_cases/batch_size

x = T.matrix('x')  # the data is presented as rasterized images
index = T.lscalar()    # index to a [mini]batch

r = RBM(input = x, num_vis = num_vis, num_hid = num_hid)

cost, free_en, gparam, updates = r.get_cost_updates(lr=lr_giv, persistent=persistent_giv, k=k_giv)

train_rbm = theano.function([index], [cost, free_en, gparam],
           updates=updates,
           givens=[(x, data_sh[index * batch_size: (index + 1) * batch_size])],
           name='train_rbm')

#need_train=True
need_train=False
if need_train:
    for ep in xrange(0,max_epoch):
        for i in xrange(0,num_batches):
            cost, cur_free_en, cur_gparam = train_rbm(i)
            print "Epoch # %d:%d cost: %f free energy: %f grad: %f" % (ep, i, cost, cur_free_en, cur_gparam)

    save_file = open('sentim_500', 'wb')  # this will overwrite current contents
    cPickle.dump(r.W.get_value(borrow=True), save_file, -1)  # the -1 is for HIGHEST_PROTOCOL
    cPickle.dump(r.vbias.get_value(borrow=True), save_file, -1)  # .. and it triggers much more efficient
    cPickle.dump(r.hbias.get_value(borrow=True), save_file, -1)  # .. storage than numpy's default
    save_file.close()
else:
    save_file = open('sentim_500')
    r.W.set_value(cPickle.load(save_file), borrow=True)
    r.vbias.set_value(cPickle.load(save_file), borrow=True)
    r.hbias.set_value(cPickle.load(save_file), borrow=True)



hid = r.prop_up(data_sh)

# next hyp params:
num_vis = 500
num_hid = 2
lr_giv = 0.01
persistent_giv = False
k_giv = 1

y = T.matrix('y')
r_line = RBMBinLine(input = y, num_vis = num_vis, num_hid = num_hid)

cost, updates = r_line.get_cost_updates(lr=lr_giv, weight_cost = 0.0002,
                                                         num_cases = num_cases, momentum = 0.5, 
                                                         persistent=persistent_giv, k=k_giv)

train_rbm_line = theano.function([index], cost,
           updates=updates,
           givens=[(y, hid[index * batch_size: (index + 1) * batch_size])]
           )
for ep in xrange(0,max_epoch):
    for i in xrange(0,num_batches):
        cost = train_rbm_line(0)
        print "Epoch # %d:%d cost: %f" % (ep, i, cost)

#err = get_error(r, data_sh)
#fine_tune(r, data_sh)
#err_aft = get_error(r, data_sh)
#for i in xrange(0, num_batches):
#    print "was %f now %f" % (err[i], err_aft[i])

