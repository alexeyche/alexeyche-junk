#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
import cPickle
from rbm import RBM, RBMReplSoftmax
from rbm_util import gen_name

class RBMStack():
    def __init__(self, num_vis, hid_layers_size, repl_softmax = False):
        self.input = T.matrix('input') 
        self.params = []
        self.num_layers = len(hid_layers_size)
        self.num_vis = num_vis
        self.stack = []
        num_vis_cur = self.num_vis
        input_cur = self.input
        self.isTrained = False
        for l in xrange(0, self.num_layers):
            num_hid_cur = hid_layers_size[l]
             
            if l > 0:
                input_cur = self.stack[-1].output
                num_vis_cur = self.stack[-1].num_hid
            
            if repl_softmax and l == 0:  # replicated softmax layer only for first one
                rbm = RBMReplSoftmax(input = input_cur, num_vis = num_vis_cur, num_hid = num_hid_cur)
            else:    
                rbm = RBM(input = input_cur, num_vis = num_vis_cur, num_hid = num_hid_cur)
             
            self.stack.append(rbm)

    def __getitem__(self, index):
        return self.stack[index]
    def append(self,rbm):
        self.stack.append(rbm)
        self.num_layers = len(self.stack)
    
    def pretrain_fun(self, data_sh, train_params):
        batch_size = train_params['batch_size']
        learning_rate = train_params['learning_rate']
        cd_steps = train_params['cd_steps']
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

    def pretrain(self, data_sh, train_params):
        max_epoch = train_params['max_epoch']
        num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
        fn = self.pretrain_fun(data_sh, train_params)
        for i in xrange(0,len(fn)):
            f = fn[i]
            for ep in xrange(0, max_epoch):
                for b in xrange(0, num_batches):
                    cost, cur_free_en, cur_gparam = f(b)
                    print "pretrain, layer %s, epoch # %d:%d cost: %f free energy: %f grad: %f" % (i, ep, b, cost, cur_free_en, cur_gparam)
        self.isTrained = True
      


