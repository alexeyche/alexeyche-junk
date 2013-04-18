#!/usr/bin/env python

import theano
import theano.tensor as T
import numpy as np
import os, sys
from theano.tensor.shared_randomstreams import RandomStreams


def logsum(x):
    alpha = T.max(x) - T.log(np.finfo(theano.config.floatX).max)/2
    return alpha + T.log(T.sum(T.exp(x-alpha))) 


class AIS(object):
    def __init__(self, rbm, betas = None, numruns=100, data = None, mean_field = False):
        self.numpy_rng = np.random.RandomState(1)
        self.theano_rng = RandomStreams(self.numpy_rng.randint(2 ** 30))

        self.model = rbm
        if betas != None:
            self.betas = betas
        else:
            self.betas = np.asarray(np.concatenate([np.arange(0,0.5,1e-03), np.arange(0.5,0.9,1e-04),np.arange(0.9,1,1e-04)]), dtype=theano.config.floatX)
        self.numruns = numruns
        if data != None:
            base_vbias_init = self.data_mean(data)
        else:            
            base_vbias_init = np.zeros(self.model.num_vis, dtype=theano.config.floatX)
        
        self.base_vbias = theano.shared(value=base_vbias_init, name='base_vbias', borrow=True)
        
        if mean_field:
            self.mean_field = 1
        else:
            self.mean_field = 0

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
    
    def est_log_part_fun(self):
        # init first visible data 
        v_mean = T.nnet.sigmoid(self.base_vbias) 
        v_mean_rep = T.tile(v_mean, (self.numruns,)).reshape((self.numruns, self.model.num_vis))
        v = self.theano_rng.binomial(size=v_mean_rep.shape, n=1, p=v_mean_rep, dtype=theano.config.floatX)
        # init logw with beta = 0
        logw = - self.log_p_k(v, 0.) 
        
        [logw_list, vs], updates = theano.scan(self.ais_step, sequences = self.betas[1:], outputs_info = [logw, v])
        
        logw = logw_list[-1]
        v = vs[-1]
        
        logw += self.log_p_k(v, 1)            
        r = logsum(logw) - T.log(self.numruns) 

        log_z_base = T.sum(T.log(1+T.exp(self.base_vbias))) + (self.model.num_hid)*T.log(2)
        log_z_est = r + log_z_base
        
        perform_fun = theano.function([], [logw_list, vs, log_z_est], updates=updates)

        return perform_fun()

def log_prob_data(model, log_z, data):
    return model.free_energy(data) - log_z


class AIS_RS(AIS):
    def sample_h_given_v(self, v, beta, D):
        pre_sigmoid_activation = beta * (T.dot(v, self.model.W) + T.outer(D, self.model.hbias))
        h_mean = T.nnet.sigmoid(pre_sigmoid_activation)
        h_sample = self.theano_rng.binomial(size=h_mean.shape, n=1, p=h_mean, dtype=theano.config.floatX)
        return h_sample

    def sample_v_given_h(self, h, beta, D):
        pre_softmax_activation = (1-beta) * self.base_vbias + beta * (T.dot(h, self.model.W.T) + self.model.vbias)
        v_mean = T.nnet.softmax(pre_softmax_activation)
        
        #v_sample = D.dimshuffle(0,'x') * v_mean
        v_samples, updates = theano.scan(fn=self.multinom_sampler,non_sequences=[v_mean, D], n_steps=5)        
        self.updates = updates
        v_sample = v_samples[-1]
        
        return v_sample

    def multinom_sampler(self, probs, D):
        v_sample = self.theano_rng.multinomial(n=D, pvals=probs, dtype=theano.config.floatX)
        return v_sample

    def free_energy_base(self, v, beta):
        return -beta * T.dot(v, self.base_vbias.T)

    def free_energy(self, v, beta, D):
        return -beta * T.dot(v, self.model.vbias.T) - T.sum(T.log(1+T.exp( beta * (T.dot(v,self.model.W) + T.outer(D,self.model.hbias)) )),axis=1)
 
    def log_p_k(self, v, beta, D):
        return -self.free_energy_base(v, 1-beta) - self.free_energy(v, beta, D)
  
    def ais_step(self, beta, logw, v):
        D = T.sum(v, axis=1)
        logw += self.log_p_k(v, beta, D)
        h = self.sample_h_given_v(v, beta, D)
        v_next = self.sample_v_given_h(h, beta, D, self.mean_field)
        logw -= self.log_p_k(v_next, beta, D)
        return logw, v_next, D
    
    def est_log_part_fun(self):
        # init first visible data 
        v_mean = T.nnet.softmax(self.base_vbias)[0]
        v_mean_rep = T.tile(v_mean, (self.numruns,)).reshape((self.numruns, self.model.num_vis))
        D = T.tile(T.sum(self.base_vbias, axis=0).dimshuffle('x'), (self.numruns,))
        v_samples, updates = theano.scan(fn=self.multinom_sampler,non_sequences=[v_mean_rep, D], n_steps=10)        
       
        v = v_samples[-1]
        # init logw with beta = 0
        logw = - self.log_p_k(v, 0., D) 
        
        [logw_list, vs, Ds], updates = theano.scan(self.ais_step, sequences = self.betas[1:], outputs_info = [logw, v, None])
        
        logw = logw_list[-1]
        v = vs[-1]
        D = Ds[-1] 
        
        logw += self.log_p_k(v, 1, D)            
        r = logsum(logw) - T.log(self.numruns) 

        log_z_base = T.sum(T.log(1+T.exp(self.base_vbias))) + (self.model.num_hid)*T.log(2)
        log_z_est = r + log_z_base
        
        perform_fun = theano.function([], log_z_est, updates=updates)

        return perform_fun()

