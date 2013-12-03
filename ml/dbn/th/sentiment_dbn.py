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
from dbn import DBN
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

#data_proportions = [70,15,15]

num_train_data = 5000 #int(round(data_proportions[0] * num_cases/100))
num_valid_data = 1000 #int(round(data_proportions[1] * num_cases/100))
num_test_data = num_cases - num_train_data - num_valid_data
train_data = data[0:num_train_data]
train_data_t = data_target[0:num_train_data]
valid_data = data[num_train_data:num_train_data+num_valid_data]
valid_data_t = data_target[num_train_data:num_train_data+num_valid_data]
test_data = data[num_train_data+num_valid_data:]
test_data_t = data_target[num_train_data+num_valid_data:]

def make_theano_dataset(datasets):
    data = datasets[0]
    data_t = datasets[1]
    data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
    data_t_sh = theano.shared(np.asarray(data_t, dtype=theano.config.floatX), borrow=True)
    return data_sh, T.cast(data_t_sh, 'int32')

train_data_sh, train_data_t_sh = make_theano_dataset((train_data, train_data_t))
valid_data_sh, valid_data_t_sh = make_theano_dataset((valid_data, valid_data_t))
test_data_sh, test_data_t_sh = make_theano_dataset((test_data, test_data_t))

datasets = [(train_data_sh, train_data_t_sh),(valid_data_sh, valid_data_t_sh),(test_data_sh, test_data_t_sh)]
rbm_stack = RBMStack(num_dims, [500])
dbn = DBN(rbm_stack, 2)

batch_size = 100
max_epoch = 10
train_params = {'batch_size' : batch_size, 'learning_rate' : 0.01, 'cd_steps' : 2, 'max_epoch' : max_epoch, 'persistent' : True, 'finetune_learning_rate' : 0.1 }

pre_fn = dbn.pretrain_fun(train_data_sh, train_params)

num_batches = train_data_sh.get_value(borrow=True).shape[0]/batch_size
for f in pre_fn:
    for ep in xrange(0, max_epoch):
        for b in xrange(0, num_batches):
            cost, cur_free_en, cur_gparam = f(b)
            print "Epoch # %d:%d cost: %f free energy: %f grad: %f" % (ep, b, cost, cur_free_en, cur_gparam)
   
train, valid, test = dbn.finetune_fun(datasets, train_params)
validation_frequency = 200
for ep in xrange(0, 30):
    for b in xrange(0, num_batches):
        cost = train(b)
        print "Epoch # %d:%d cost: %f" % (ep, b, cost)
        iter = ep * num_batches+ b
        if (iter + 1) % validation_frequency == 0:
            validation_losses = valid()
            this_validation_loss = np.mean(validation_losses)
            print('epoch %i, minibatch %i/%i, validation error %f %%' % (ep, b + 1, num_batches, this_validation_loss * 100.))
#num_datasets = [ num_train_data, num_valid_data, num_test_data ]

#data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
#data_target_sh = theano.shared(np.asarray(data_target, dtype=theano.config.floatX), borrow=True)

