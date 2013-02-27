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

class DBN(AutoEncoder):
    def __init__(self, num_vis, hid_layers_size, num_out):
        self.x = T.matrix('x')  
        self.y = T.ivector('y')
        self.params = []
        self.num_layers = len(hid_layers_size)
        self.num_vis = num_vis
        self.num_out = num_out
        self.stack = []
        num_vis_cur = self.num_vis
        input_cur = self.input
        for l in xrange(0, self.num_layers):
            num_hid_cur = hid_layers_size[l]
             
            if l > 0:
                input_cur = self.stack[-1].output
                num_vis_cur = self.stack[-1].num_hid

            rbm = RBM(input = input_cur, num_vis = num_vis_cur, num_hid = num_hid_cur)
             
            self.stack.append(rbm)
            self.params.extend(rbm.params)
        
        # init params for LogLayer 
        self.W_ll = theano.shared(value=numpy.zeros((self.stack[-1].num_hid, self.num_out), dtype=theano.config.floatX), name='W_ll', borrow=True)
        self.b_ll = theano.shared(value=numpy.zeros((self.num_out,), dtype=theano.config.floatX), name='b_ll', borrow=True)
        self.params.extend([self.W_ll, self.b_ll])
        
        self.p_y_given_x = T.nnet.softmax(T.dot(self.stack[-1].output, self.W_ll) + self.b_ll)
        self.y_pred = T.argmax(self.p_y_given_x, axis=1)

    def pretrain_fun(self, data_sh, train_params):
        batch_size = train_params['batch_size']
        learning_rate = train_params['learning_rate']
        cd_steps = train_params['cd_steps']
        learning_rate_line = train_params['learning_rate_line'] 
        persistent = train_params['persistent']
        
        pretrain_fns = [] 
        index = T.lscalar('index')  # index to a minibatch
        for l in xrange(0, self.num_layers):
            rbm = self.stack[l]
            if persistent:
                persistent_chain = theano.shared(np.zeros((batch_size, rbm.num_hid), dtype=theano.config.floatX), borrow=True)
            else:
                persistent_chain = None     

            cost, free_en, gparam, updates = rbm.get_cost_updates(lr=learning_rate, persistent=persistent_chain, k=cd_steps)
            
            train_rbm_f = theano.function([index], [cost, free_en, gparam],
                   updates=updates,
                   givens=[(self.input, data_sh[index * batch_size: (index + 1) * batch_size])])
            
            pretrain_fns.append(train_rbm_f)
        return pretrain_fns            
    
    def get_output(self):
        output = self.stack[-1].output # from the top
        for l in reversed(xrange(0,self.num_layers)):
            _, output = self.stack[l].prop_down(output)
        return output
    
    def negative_log_likelihood(self):
        return -T.mean(T.log(self.p_y_given_x)[T.arange(self.y.shape[0]), self.y])
    def errors(self):
        return T.mean(T.neq(self.y_pred, self.y))
  
    def finetune_fun(self, datasets, train_params):
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

