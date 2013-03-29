#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
import cPickle
from rbm import RBM
from rbm_rs import RBMReplSoftmax
from rbm_util import gen_name

CACHE_PATH="/mnt/yandex.disk/models/rs"

class RBMStack():
    def __init__(self, num_vis = None, hid_layers_size = [], bottomRBMtype = None, add_opts = {}):
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

            rbm = None 
            if l == 0:  # replicated softmax layer only for first one
                if bottomRBMtype == None or bottomRBMtype == RBM:
                    rbm = RBM(input = input_cur, num_vis = num_vis_cur, num_hid = num_hid_cur)
                if bottomRBMtype == RBMReplSoftmax:
                    rbm = RBMReplSoftmax(input = input_cur, num_vis = num_vis_cur, num_hid = num_hid_cur)
            else:
                rbm = RBM(input = input_cur, num_vis = num_vis_cur, num_hid = num_hid_cur)

            assert(rbm)
            self.stack.append(rbm)

    def __getitem__(self, index):
        return self.stack[index]

    def append(self,rbm):
        self.stack.append(rbm)
        self.num_layers = len(self.stack)
    
    def pretrain_fun(self, data_sh, train_params):
        ep_inc = train_params['ep_inc']
        persistent = train_params['persistent_on']
        batch_size = train_params['batch_size']

        pretrain_fns = [] 
        index = T.lscalar('index')  # index to a minibatch
        for l in xrange(0, self.num_layers):
            rbm = self.stack[l]
            if rbm.need_train == False:
                continue
            if persistent:
                train_params['persistent'] = theano.shared(np.zeros((batch_size, rbm.num_hid), dtype=theano.config.floatX), borrow=True)
            else:
                train_params['persistent'] = None
            cost, free_en, gparam, updates = rbm.get_cost_updates(train_params)
            updates.update([(rbm.epoch_ratio, rbm.epoch_ratio + ep_inc)])

            train_rbm_f = theano.function([index], [cost, free_en, gparam],
                   updates=updates,
                   givens=[(rbm.input, data_sh[index * batch_size: (index + 1) * batch_size])])
            
            pretrain_fns.append(train_rbm_f)
        return pretrain_fns            

    def pretrain(self, data_sh, train_params):
        max_epoch = train_params['max_epoch']
        num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
        train_params['ep_inc'] = np.float32(1.0/(num_batches*max_epoch))
        
        fn = self.pretrain_fun(data_sh, train_params)
        for i in xrange(0,len(fn)):
            f = fn[i]
            mean_cost = []
            mean_cost_last = 0
            for ep in xrange(0, max_epoch):
                for b in xrange(0, num_batches):
                    cost, cur_free_en, cur_gparam = f(b)
                    mean_cost.append(cost)
                    epoch_ratio = self.stack[i].epoch_ratio.get_value(borrow=True)
                    print "pretrain(%3.1f), layer %s, epoch # %d:%d last mean cost %2.2f (cost: %f) free energy: %f grad: %f" % (epoch_ratio*100,0, ep, b, mean_cost_last, cost, cur_free_en, cur_gparam)
                mean_cost_last = np.mean(mean_cost)
                mean_cost = []

            self.stack[i].save_model(CACHE_PATH)
        self.isTrained = True
      


