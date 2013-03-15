#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from theano.tensor.shared_randomstreams import RandomStreams


class CRBM(object):
    def __init__(self, input = None, history = None, num_vis, num_hid, n_delay):
        self.numpy_rng = np.random.RandomState(1)
        self.theano_rng = RandomStreams(self.numpy_rng.randint(2 ** 30))
        self.num_vis = num_vis 
        self.num_hid = num_hid
        
        self.input = input
        if input is None:
            self.input = T.matrix('input')
        initial_W = np.asarray(0.01 * self.numpy_rng.randn(self.num_vis, self.num_hid), dtype=theano.config.floatX)
        self.W = theano.shared(value=initial_W, name='W', borrow=True)
        self.cond_params_uv = []
        self.cond_params_uh = []
        for(i in xrange(0, n_delay):
            initial_W_uv = np.asarray(0.01 * self.numpy_rng.randn(self.num_vis, self.num_vis), dtype=theano.config.floatX)
            W_uv = theano.shared(value=initial_W, name='W_uv', borrow=True)
            initial_W_uh = np.asarray(0.01 * self.numpy_rng.randn(self.num_vis, self.num_hid), dtype=theano.config.floatX)
            W_uh = theano.shared(value=initial_W, name='W_uh', borrow=True)
            self.cond_params_uv.append(W_uv)
            self.cond_params_uh.append(W_uh)

        self.hbias = theano.shared(value=np.zeros(self.num_hid, dtype=theano.config.floatX), name='hbias', borrow=True)
        self.vbias = theano.shared(value=np.zeros(self.num_vis, dtype=theano.config.floatX), name='vbias', borrow=True)
        self.batch_size = input.shape(0)
        self.vbias_c = theano.shared(value=np.zeros((self.batch_size, self.num_vis), dtype=theano.config.floatX), name='vbias_c', borrow=True)
        self.hbias_c = theano.shared(value=np.zeros((self.batch_size, self.num_hid), dtype=theano.config.floatX), name='hbias_c', borrow=True)

    def prop_up(self, vis):
        pre_sigmoid_activation = T.dot(vis, self.W) + self.hbias
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]

    def prop_down(self, hid):
        pre_sigmoid_activation = T.dot(hid, self.W.T) + self.vbias
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]
    
    def free_energy(self, v_sample):
        wx_b = T.dot(v_sample, self.W) + self.hbias + self.hbias_c
        biased_sample = v_sample + self.vbias_c
        vbias_term = T.dot(biased_sample, self.vbias)  
        hidden_term = T.sum(T.log(1 + T.exp(wx_b)), axis=1)
        return -hidden_term - vbias_term
       



