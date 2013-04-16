#!/usr/bin/env python

import theano
import theano.tensor as T
import numpy as np
import os
from theano.tensor.shared_randomstreams import RandomStreams


class AIS(object):
    def __init__(self, rbm, betas = None, numruns=100, data = None):
        self.numpy_rng = np.random.RandomState(1)
        self.theano_rng = RandomStreams(self.numpy_rng.randint(2 ** 30))

        self.model = rbm
        if betas:
            self.betas = betas
        else:
            self.betas = np.asarray(np.concatenate([np.arange(0,0.5,1e-03), np.arange(0.5,0.9,1e-04),np.arange(0.9,1,1e-04)]), dtype=theano.config.floatX)
        self.numruns = numruns
        if data != None:
            base_vbias_init = self.data_mean(data)
        else:            
            base_vbias_init = np.zeros(self.model.num_vis, dtype=theano.config.floatX)
        
        self.base_vbias = theano.shared(value=base_vbias_init, name='base_vbias', borrow=True)

    def data_mean(self,data):
        data_m = np.mean(data,axis=0)
        return data_m
    
    def sample_h_given_v(self, v, beta):
        pre_sigmoid_activation = beta * (T.dot(v, self.model.W) + self.model.hbias)
        h_mean = T.nnet.sigmoid(pre_sigmoid_activation)
        h_sample = self.theano_rng.binomial(size=h_mean.shape, n=1, p=h_mean, dtype=theano.config.floatX)
        return h_sample

    def sample_v_given_h(self, h, beta):
        pre_sigmoid_activation = (1-beta) * self.base_vbias + beta * (T.dot(h, self.model.W.T) + self.model.vbias)
        v_mean = T.nnet.sigmoid(pre_sigmoid_activation)
        v_sample = self.theano_rng.binomial(size=v_mean.shape, n=1, p=v_mean, dtype=theano.config.floatX)
        return v_sample

    def free_energy_base(self, v, beta):
        return -beta * T.dot(v, self.base_vbias.T)

    def free_energy(self, v, beta):
        return -beta * T.dot(v, self.model.vbias.T) - T.sum(T.log(1+T.exp( beta * (T.dot(v,self.model.W) + self.model.hbias) )),axis=1)

    def log_p_k(self, v, beta):
        return -self.free_energy_base(v, 1-beta) - self.free_energy(v, beta)
   
    def ais_step(self, beta, logw, v):
        logw += self.log_p_k(v, beta)
        h = self.sample_h_given_v(v, beta)
        v_next = self.sample_v_given_h(h, beta)
        logw -= self.log_p_k(v_next, beta)
        return logw, v_next
    
    def perform(self):
        # init first visible data 
        v_mean = T.nnet.sigmoid(self.base_vbias) 
        v_mean_rep = T.tile(v_mean, (self.numruns,)).reshape((self.numruns, self.model.num_vis))
        v = self.theano_rng.binomial(size=v_mean_rep.shape, n=1, p=v_mean_rep, dtype=theano.config.floatX)
        # init logw with beta = 0
        logw = - self.log_p_k(v, 0)

        [logw_list, vs], updates = theano.scan(self.ais_step, sequences = [self.betas[1:]], outputs_info = [logw, v])
        logw = logw_list[-1]
        logw += self.log_p_k(v, 1)            
        
        return theano.function([], logw, updates=updates)
