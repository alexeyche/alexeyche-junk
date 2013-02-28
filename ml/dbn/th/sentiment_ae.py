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

from ae import AutoEncoder, save_to_file, load_from_file, print_top_to_file
from rbm import RBMBinLine
from rbm_stack import RBMStack

csvfile = '/home/alexeyche/my/dbn/kaggle_sentiment/training_feat_proc.csv'
#csvfile = '/home/alexeyche/prog/alexeyche-junk/ml/dbn/sentiment/training_feat_proc.csv'
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
max_epoch = 30
train_params = {'batch_size' : batch_size, 'learning_rate' : 0.01, 'cd_steps' : 5, 'max_epoch' : max_epoch, 'persistent' : True, 'learning_rate_line' : 0.001, 'finetune_learning_rate' : 0.1 }

stack_rbm = RBMStack(num_vis = num_vis, hid_layers_size = [500])
sent_ae = AutoEncoder(stack_rbm)
val = sent_ae.finetune_cost()

index = T.lscalar('index')  # index to a minibatch
fun_cost = theano.function([index],val,givens=[(sent_ae.input, data_sh[index * batch_size: (index + 1) * batch_size])])
cost_before = []
for b in xrange(0, num_batches):
    cost_before.append(fun_cost(b))

need_train = True
if need_train:
    fn = sent_ae.pretrain_fun(data_sh, train_params)
    for f in fn:
        for ep in xrange(0, max_epoch):
            for b in xrange(0, num_batches):
                cost, cur_free_en, cur_gparam = f(b)
                print "Epoch # %d:%d cost: %f free energy: %f grad: %f" % (ep, b, cost, cur_free_en, cur_gparam)

    save_to_file(sent_ae)
    print "ae saved"
    perm_cut = range(0,999)
    print_top_to_file(sent_ae, "pre_train", data_sh, data_target, perm_cut)
    for b in xrange(0, num_batches):
        cost = fun_cost(b)    
        print "before pre train: %f after pre train: %f" % (cost_before[b], cost)
else:
    load_from_file(sent_ae)
    # learn 3 rbm again
    #sent_ae.stack[-1].reset_params()
    #fn = sent_ae.pretrain_fun(data_sh, train_params)
    #for ep in xrange(0, max_epoch):
    #    for b in xrange(0, num_batches):
    #        cost, cur_free_en, cur_gparam = fn[2](b)
    #        print "Epoch # %d:%d cost: %f free energy: %f grad: %f" % (ep, b, cost, cur_free_en, cur_gparam)

need_finetune=True
if need_finetune:
    fine_tune = sent_ae.finetune_fun(data_sh, train_params)
    for ep in xrange(0, max_epoch):
        for b in xrange(0, num_batches):
            cost = fine_tune(b)
            print "Epoch # %d:%d cost: %f" % (ep, b, cost)

    perm_cut = range(0,999)
    print_top_to_file(sent_ae, "fine_tune", data_sh, data_target, perm_cut)
    save_to_file(sent_ae)
else:
    load_from_file(sent_ae)

    
