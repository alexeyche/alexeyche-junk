#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from theano.tensor.shared_randomstreams import RandomStreams


class CRBM(object):
    def __init__(self, input = None, history = None, num_vis = 10, num_hid = 15, n_delay = 3):
        self.numpy_rng = np.random.RandomState(1)
        self.theano_rng = RandomStreams(self.numpy_rng.randint(2 ** 30))
        self.num_vis = num_vis 
        self.num_hid = num_hid
        self.n_delay = n_delay

        self.input = input
        self.history = history
        if input is None:
            self.input = T.matrix('input')
        if history is None:
            self.history = T.tensor3('history')

        initial_W = np.asarray(0.01 * self.numpy_rng.randn(self.num_vis, self.num_hid), dtype=theano.config.floatX)
        self.W = theano.shared(value=initial_W, name='W', borrow=True)
        self.cond_params_uv = []
        self.cond_params_uh = []
        for i in xrange(0, n_delay):
            initial_W_uv = np.asarray(0.01 * self.numpy_rng.randn(self.num_vis, self.num_vis), dtype=theano.config.floatX)
            W_uv = theano.shared(value=initial_W, name='W_uv', borrow=True)
            initial_W_uh = np.asarray(0.01 * self.numpy_rng.randn(self.num_vis, self.num_hid), dtype=theano.config.floatX)
            W_uh = theano.shared(value=initial_W, name='W_uh', borrow=True)
            self.cond_params_uv.append(W_uv)
            self.cond_params_uh.append(W_uh)

        self.hbias = theano.shared(value=np.zeros(self.num_hid, dtype=theano.config.floatX), name='hbias', borrow=True)
        self.vbias = theano.shared(value=np.zeros(self.num_vis, dtype=theano.config.floatX), name='vbias', borrow=True)
        self.vbias_c, self.hbias_c = self.calc_bias_c()

    def prop_up(self, vis):
        pre_sigmoid_activation = T.dot(vis, self.W) + self.hbias + self.hbias_c
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]

    def prop_down(self, hid, vbias_c):
        pre_sigmoid_activation = T.dot(hid, self.W.T) + self.vbias + self.vbias_c
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]
    
    def calc_bias_c(self):
        vbias_c = hbias_c = None
        for i in xrange(0, self.n_delay):
            W_uv = self.cond_params_uv[i]
            W_uh = self.cond_params_uh[i]
            vbias_c_t = T.dot(self.history[i,:,:], W_uv)                    
            hbias_c_t = T.dot(self.history[i,:,:], W_uh)                    
            if vbias_c:
                vbias_c += vbias_c_t
            else:
                vbias_c = vbias_c_t
            if hbias_c:
                hbias_c += hbias_c_t
            else:
                hbias_c = hbias_c_t
        return (vbias_c, hbias_c) 
    
    def free_energy(self, v_sample):
        wx_b = T.dot(v_sample, self.W) + self.hbias + self.hbias_c
        biased_sample = v_sample + self.vbias_c
        vbias_term = T.dot(biased_sample, self.vbias)  
        hidden_term = T.sum(T.log(1 + T.exp(wx_b)), axis=1)
        return -hidden_term - vbias_term
       



