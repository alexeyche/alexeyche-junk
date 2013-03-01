#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os, sys
from theano.tensor.shared_randomstreams import RandomStreams
from theano import ProfileMode
import cPickle

from ae import AutoEncoder, print_top_to_file
from rbm import RBMBinLine
from rbm_stack import RBMStack, save_to_file, load_from_file, gen_name
from dbn import DBN

if sys.argv[1] == '1':
    hid_layers_size = [500]
if sys.argv[1] == '2':
    hid_layers_size = [500,250]
if sys.argv[1] == '3':
    hid_layers_size = [500,250,100]

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

# for autoencoder
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True) 
num_train_data = 6000
num_valid_data = 1000 #int(round(data_proportions[1] * num_cases/100))
#num_test_data = num_cases - num_train_data - num_valid_data
train_data = data[0:num_train_data]
train_data_t = data_target[0:num_train_data]
valid_data = data[num_train_data:num_train_data+num_valid_data]
valid_data_t = data_target[num_train_data:num_train_data+num_valid_data]
test_data = None # data[num_train_data+num_valid_data:]
test_data_t = None # data_target[num_train_data+num_valid_data:]

def make_theano_dataset(datasets):
    data = datasets[0]
    data_t = datasets[1]
    data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
    data_t_sh = theano.shared(np.asarray(data_t, dtype=theano.config.floatX), borrow=True)
    return data_sh, T.cast(data_t_sh, 'int32')

train_data_sh, train_data_t_sh = make_theano_dataset((train_data, train_data_t))
valid_data_sh, valid_data_t_sh = make_theano_dataset((valid_data, valid_data_t))
test_data_sh, test_data_t_sh = (None, None) #make_theano_dataset((test_data, test_data_t))

datasets = [(train_data_sh, train_data_t_sh),(valid_data_sh, valid_data_t_sh),(test_data_sh, test_data_t_sh)]

batch_size = 100
max_epoch = 50
train_params = {'batch_size' : batch_size, 'learning_rate' : 0.1, 'cd_steps' : 10, 'max_epoch' : max_epoch, 'persistent' : True, 'cd_steps_line' : 1, 'learning_rate_line' : 0.001, 'finetune_learning_rate' : 0.1 }

stack_rbm = RBMStack(num_vis = num_vis, hid_layers_size = hid_layers_size)

if not load_from_file(stack_rbm, train_params):
    stack_rbm.pretrain(data_sh, train_params)
    save_to_file(stack_rbm, train_params)

# fun with AutoEncoder

ae = AutoEncoder(stack_rbm)
ae.pretrain(data_sh, train_params)

print_top_to_file(ae, train_params, "pre_train", data_sh, data_target, range(0,1499))
ae.finetune(data_sh, train_params)
print_top_to_file(ae, train_params, "fine_tune", data_sh, data_target, range(0,1499))


