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

from ae import AutoEncoder, print_top_to_file
from rbm import RBMBinLine
from rbm_stack import RBMStack, save_to_file, load_from_file, gen_name

#csvfile = '/home/alexeyche/my/dbn/kaggle_sentiment/training_feat_proc.csv'
csvfile = '/home/alexeyche/prog/alexeyche-junk/ml/dbn/sentiment/training_feat_proc.csv'
#csvfile = 'two_binom.csv'
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

batch_size = 70
num_batches = num_cases/batch_size
max_epoch = 50
train_params = {'batch_size' : batch_size, 'learning_rate' : 0.01, 'cd_steps' : 5, 'max_epoch' : max_epoch, 'persistent' : True, 'learning_rate_line' : 0.0001, 'finetune_learning_rate' : 0.1 }

stack_rbm = RBMStack(num_vis = num_vis, hid_layers_size = [500])
if not load_from_file(stack_rbm, train_params):
    stack_rbm.pretrain(data_sh, train_params)
    save_to_file(stack_rbm, train_params)


print_top_to_file(sent_ae, "pre_train", data_sh, data_target, range(0,999))
fine_tune = sent_ae.finetune_fun(data_sh, train_params)
for ep in xrange(0, max_epoch):
    for b in xrange(0, num_batches):
        cost = fine_tune(b)
        print "Epoch # %d:%d cost: %f" % (ep, b, cost)

print_top_to_file(sent_ae, "fine_tune", data_sh, data_target, range(0,999))

    
