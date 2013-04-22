#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams

import rpy2.robjects as ro
from rpy2.robjects.packages import importr

from rbm import RBM, RBMBinLine
from rbm_rs import RBMReplSoftmax
from rbm_util import gen_name

    
class AutoEncoder(object):
    def __init__(self, stack, num_out = 2, top_line = True):
        self.input = stack[0].input
        self.output = T.ivector('output')
        
        self.stack = stack
        self.num_vis = stack[0].num_vis
        if top_line: 
            top_rbm = RBMBinLine(input = self.stack[-1].output, num_vis = self.stack[-1].num_hid, num_hid = num_out)  
        else:                 
            top_rbm = RBM(input = self.stack[-1].output, num_vis = self.stack[-1].num_hid, num_hid = num_out)  

        self.stack.append(top_rbm)
        self.params = []
        for rbm in stack:
            self.params.extend(rbm.params)
    
        self.num_layers = self.stack.num_layers

    def pretrain_fun(self, data_sh, train_params):
        ep_inc = train_params['ep_inc']
        persistent = train_params['persistent_on']
        batch_size = train_params['batch_size']
        max_epoch = train_params['max_epoch']
        num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
        train_params['ep_inc'] = np.float32(1.0/(num_batches*max_epoch))

        if persistent and type(self.stack[-1]) is RBM:
            train_params['persistent'] = theano.shared(np.zeros((batch_size, self.stack[-1].num_hid), dtype=theano.config.floatX), borrow=True)
        else:
            train_params['persistent'] = None            

        index = T.lscalar('index')  # index to a minibatch
        cost, free_en, gparam, updates = self.stack[-1].get_cost_updates(train_params)
  
        updates.update([(self.stack[-1].epoch_ratio, self.stack[-1].epoch_ratio + ep_inc)])
        train_rbm_f = theano.function([index], [cost, free_en, gparam],
               updates=updates,
               givens=[(self.input, data_sh[index * batch_size: (index + 1) * batch_size])])
            
        return train_rbm_f
    
    def pretrain(self, data_sh, data_valid_sh, train_params):
        if self.stack.need_train:
            list(self.stack.pretrain(data_sh, data_valid_sh, train_params))
        
        max_epoch = train_params['max_epoch']
        num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
        fn = self.pretrain_fun(data_sh, train_params)
        
        for ep in xrange(0, max_epoch):
            for b in xrange(0, num_batches):
                cost, cur_free_en, cur_gparam = fn(b)
                print "pretrain, top layer, epoch # %d:%d cost: %f free energy: %f grad: %f" % (ep, b, cost, cur_free_en, cur_gparam)

    def get_output(self):
        output = self.stack[-1].output # from the top
        for l in reversed(xrange(0,self.num_layers)):
            _, output = self.stack[l].prop_down(output)
        if type(self.stack[0]) is RBMReplSoftmax:
            D = T.sum(self.input, axis=1).dimshuffle(0,'x')
            output = D*output 
        return output

    def finetune_cost(self):
        output = self.get_output()
        return T.mean(T.sum(T.sqr(self.input - output), axis=1))
    
    def finetune_fun(self, data_sh, train_params):
        batch_size = train_params['batch_size']
        learning_rate = train_params['finetune_learning_rate']

        index = T.lscalar('index')  # index to a [mini]batch
        cost = self.finetune_cost() # wrt to self.input
        gparams = T.grad(cost, self.params)
        updates = []
        for param, gparam in zip(self.params, gparams):
            updates.append((param, param - gparam * learning_rate))
        train_fn = theano.function([index], cost, updates=updates,
                                  givens=[ (self.input, data_sh[index * batch_size: (index + 1) * batch_size])])

        return train_fn
    
    def finetune(self, data_sh, train_params):
        max_epoch = train_params['max_epoch']
        num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
        fine_tune = self.finetune_fun(data_sh, train_params)
        for ep in xrange(0, max_epoch):
            for b in xrange(0, num_batches):
                cost = fine_tune(b)
                print "Finetune, epoch # %d:%d cost: %f" % (ep, b, cost)



