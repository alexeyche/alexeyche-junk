#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
import cPickle

import rpy2.robjects as ro
from rpy2.robjects.packages import importr

from rbm import RBM, RBMBinLine
from rbm_util import gen_name

    
class AutoEncoder(object):
    def __init__(self, rbms, num_out = 2):
        self.input = rbms[0].input
        self.output = T.ivector('output')
        
        self.stack = rbms 
        self.num_vis = rbms[0].num_vis
        
        top_rbm = RBMBinLine(input = self.stack[-1].output, num_vis = self.stack[-1].num_hid, num_hid = num_out)  
                     
        self.stack.append(top_rbm)
        self.params = []
        for rbm in rbms:
            self.params.extend(rbm.params)
    
        self.num_layers = self.stack.num_layers

    def pretrain_fun(self, data_sh, train_params):
        funcs = []
        if not self.stack.isTrained:
            funcs = self.stack.pretrain_fun(data_sh, train_params)
        learning_rate_line = train_params['learning_rate_line'] 
        persistent = train_params['persistent']
        cd_steps = train_params['cd_steps_line']
        batch_size = train_params['batch_size']
        num_cases = data_sh.get_value(borrow=True).shape[0]

        #if persistent:
        #    persistent_chain = theano.shared(np.zeros((batch_size, self.stack[-1].num_hid), dtype=theano.config.floatX), borrow=True)
        #else:
        persistent_chain = None            
        index = T.lscalar('index')  # index to a minibatch
        cost, free_en, gparam, updates = self.stack[-1].get_cost_updates(lr=learning_rate_line, num_cases=num_cases, persistent=persistent_chain, k=cd_steps)

        train_rbm_f = theano.function([index], [cost, free_en, gparam],
               updates=updates,
               givens=[(self.input, data_sh[index * batch_size: (index + 1) * batch_size])])
            
        funcs.append(train_rbm_f)
        return funcs
    
    def get_output(self):
        output = self.stack[-1].output # from the top
        for l in reversed(xrange(0,self.num_layers)):
            _, output = self.stack[l].prop_down(output)
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
    def pretrain(self, data_sh, train_params):
        max_epoch = train_params['max_epoch']
        num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
        fn = self.pretrain_fun(data_sh, train_params)
        for i in xrange(0,len(fn)):
            f = fn[i]
            if (i == len(fn)-1):
                max_epoch = int(round(max_epoch/2))
            for ep in xrange(0, max_epoch):
                for b in xrange(0, num_batches):
                    cost, cur_free_en, cur_gparam = f(b)
                    print "pretrain, layer %s, epoch # %d:%d cost: %f free energy: %f grad: %f" % (i, ep, b, cost, cur_free_en, cur_gparam)

    def finetune(self, data_sh, train_params):
        max_epoch = train_params['max_epoch']
        num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
        fine_tune = self.finetune_fun(data_sh, train_params)
        for ep in xrange(0, max_epoch):
            for b in xrange(0, num_batches):
                cost = fine_tune(b)
                print "Finetune, epoch # %d:%d cost: %f" % (ep, b, cost)


path="/mnt/yandex.disk/models"
def print_top_to_file(ae, train_params, name, data_sh, data_target, cases):
    ae_name = gen_name(ae, train_params)
    fileName = path + "/" + ae_name + "_" + name + ".png"

    out_line = ae.stack[-1].output
    f = theano.function([], out_line, givens=[(ae.input, data_sh[cases])])
    hid_stat = f()
    x = hid_stat[...,0].tolist()
    y = hid_stat[...,1].tolist()
    grdevices = importr('grDevices')
    grdevices.png(file=fileName, width=1024, height=1024)
    lab = ro.IntVector(data_target[cases].tolist())
    lab_col = ro.StrVector(map(lambda p: p == 0 and 'blue' or 'red', lab))
    lab_col.names = lab
    ro.r.plot(x,y, xlab = "x", ylab="y", type="n")
    ro.r.text(x,y,labels=lab, col = lab_col)
    grdevices.dev_off()

