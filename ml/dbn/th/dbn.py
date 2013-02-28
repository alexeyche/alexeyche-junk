#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
import cPickle
from ae import AutoEncoder
from rbm import RBM

class DBN(object):
    def __init__(self, rbms, num_out):
        self.stack = rbms 
        self.input = self.stack[0].input
        self.output = T.ivector('output')

        self.num_out = num_out
        self.num_layers = rbms.num_layers
        self.num_vis = self.stack[0].num_vis
        self.params = []
        for rbm in self.stack:
            self.params.extend([rbm.W, rbm.hbias])
        
        # init params for LogLayer 
        self.W_ll = theano.shared(value=np.zeros((self.stack[-1].num_hid, self.num_out), dtype=theano.config.floatX), name='W_ll', borrow=True)
        self.b_ll = theano.shared(value=np.zeros((self.num_out,), dtype=theano.config.floatX), name='b_ll', borrow=True)
        self.params.extend([self.W_ll, self.b_ll])
        
        self.p_y_given_x = T.nnet.softmax(T.dot(self.stack[-1].output, self.W_ll) + self.b_ll)
        self.y_pred = T.argmax(self.p_y_given_x, axis=1)
        self.finetune_cost = self.negative_log_likelihood()
        
    def pretrain_fun(self, data_sh, train_params):
        return self.stack.pretrain_fun(data_sh, train_params)
    
    def negative_log_likelihood(self):
        return -T.mean(T.log(self.p_y_given_x)[T.arange(self.output.shape[0]), self.output])
    
    def errors(self):
        return T.mean(T.neq(self.y_pred, self.output))
  
    def finetune_fun(self, datasets, train_params):
        batch_size = train_params['batch_size']
        learning_rate = train_params['finetune_learning_rate']
        
        (train_set_x, train_set_y) = datasets[0]
        (valid_set_x, valid_set_y) = datasets[1]
        (test_set_x, test_set_y) = datasets[2]

        # compute number of minibatches for training, validation and testing
        n_valid_batches = valid_set_x.get_value(borrow=True).shape[0]
        n_valid_batches /= batch_size
        n_test_batches = test_set_x.get_value(borrow=True).shape[0]
        n_test_batches /= batch_size

        index = T.lscalar('index')  # index to a [mini]batch
        cost = self.finetune_cost
        gparams = T.grad(cost, self.params)
        updates = []
        for param, gparam in zip(self.params, gparams):
            updates.append((param, param - gparam * learning_rate))
        
        train_fn = theano.function(inputs=[index],
              outputs=self.finetune_cost,
              updates=updates,
              givens=[(self.input, train_set_x[index * batch_size: (index + 1) * batch_size]),
                      (self.output, train_set_y[index * batch_size: (index + 1) * batch_size])])

        errors = self.errors()
        test_score_i = theano.function([index], errors,
                 givens=[(self.input, test_set_x[index * batch_size: (index + 1) * batch_size]),
                         (self.output, test_set_y[index * batch_size: (index + 1) * batch_size])])
        
        valid_score_i = theano.function([index], errors,
                 givens=[(self.input, valid_set_x[index * batch_size: (index + 1) * batch_size]),
                         (self.output, valid_set_y[index * batch_size: (index + 1) * batch_size])])

        # Create a function that scans the entire validation set
        def valid_score():
            return [valid_score_i(i) for i in xrange(n_valid_batches)]

        # Create a function that scans the entire test set
        def test_score():
            return [test_score_i(i) for i in xrange(n_test_batches)]

        return train_fn, valid_score, test_score

