#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
import cPickle

from rbm import RBM, RBMBinLine

    
class AutoEncoder(object):
    def __init__(self, input=None, num_vis = 936, hid_layers_size = [500,250,2], linear_top=True):
        self.input = input
        if input is None:
            self.input = T.matrix('input') 
        self.params = []
        self.num_layers = len(hid_layers_size)
        self.num_vis = num_vis
        self.stack = []
        num_vis_cur = self.num_vis
        input_cur = self.input
        for l in xrange(0, self.num_layers):
            num_hid_cur = hid_layers_size[l]
             
            if l > 0:
                input_cur = self.stack[-1].output
                num_vis_cur = self.stack[-1].num_hid

            if linear_top and l == self.num_layers-1:  # building top rbm
                rbm = RBMBinLine(input = input_cur, num_vis = num_vis_cur, num_hid = num_hid_cur)  
            else:
                rbm = RBM(input = input_cur, num_vis = num_vis_cur, num_hid = num_hid_cur)
             
            self.stack.append(rbm)
            self.params.extend(rbm.params)
    
    def pretrain_fun(self, data_sh, train_params):
        batch_size = train_params['batch_size']
        learning_rate = train_params['learning_rate']
        cd_steps = train_params['cd_steps']
        learning_rate_line = train_params['learning_rate_line'] 
        pretrain_fns = []
        index = T.lscalar('index')  # index to a minibatch
        for l in xrange(0, self.num_layers):
            rbm = self.stack[l]
            cost, free_en, gparam, updates = rbm.get_cost_updates(lr=learning_rate, persistent=None, k=cd_steps)
            
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
    def finetune_cost(self):
        output = self.get_output()
        return -T.mean(T.sum(output - self.input, axis=1) ** 2)

def save_to_file(ae, fileName):
    fileName_p = open(fileName, 'wb')
    for rbm in ae.stack:
        cPickle.dump(rbm.W.get_value(borrow=True), fileName_p, -1)  # the -1 is for HIGHEST_PROTOCOL
        cPickle.dump(rbm.vbias.get_value(borrow=True), fileName_p, -1)  # .. and it triggers much more efficient
        cPickle.dump(rbm.hbias.get_value(borrow=True), fileName_p, -1)  # .. storage than numpy's default

def load_from_file(ae, fileName):    
    fileName_p = open(fileName, 'r')
    for rbm in ae.stack:    
        rbm.W.set_value(cPickle.load(fileName_p), borrow=True)
        rbm.vbias.set_value(cPickle.load(fileName_p), borrow=True)
        rbm.hbias.set_value(cPickle.load(fileName_p), borrow=True)

