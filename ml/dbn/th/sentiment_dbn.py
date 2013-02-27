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

data_proportions = [70,15,15]

num_train_data = int(round(data_proportions[0] * num_cases/100))
num_valid_data = int(round(data_proportions[1] * num_cases/100))
num_test_data = num_cases - num_train_data - num_valid_data
train_data = data[0:num_train_data]
train_data_t = data_target[0:num_train_data]
valid_data = data[num_train_data:num_train_data+num_valid_data]
valid_data_t = data_target[num_train_data:num_train_data+num_valid_data]
test_data = data[num_train_data+num_valid_data:]
test_data_t = data_target[num_train_data+num_valid_data:]




#num_datasets = [ num_train_data, num_valid_data, num_test_data ]

#data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
#data_target_sh = theano.shared(np.asarray(data_target, dtype=theano.config.floatX), borrow=True)

