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

CACHE_PATH="/mnt/yandex.disk/models/rs"

class RBMReplSoftmax(RBM):
    def __init__(self, input=None, num_vis = 5, num_hid = 10, from_cache=True):
        if input is None:
            self.input = T.matrix('input')
        else:
            self.input = input

        self.numpy_rng = np.random.RandomState(1)
        self.theano_rng = RandomStreams(self.numpy_rng.randint(2 ** 30))
        self.num_vis = num_vis 
        self.num_hid = num_hid

        self.init_params()
        # initialize input layer for standalone RBM or layer0 of DBN

        self.epoch_ratio = theano.shared(np.zeros((1), dtype=theano.config.floatX), borrow=True)
        self.need_train=True
        self.D = T.sum(self.input, axis=1)
        self.params = [self.W, self.hbias, self.vbias]
        _, self.output = self.prop_up(self.input)
        
        if from_cache:
            self.restore_from_cache()

    def save_model(self, path = CACHE_PATH):
        fileName = "rbm_rs_%s_%s.model" % (self.num_vis, self.num_hid)
        fileName = os.path.join(path, fileName)
        save_file = open(fileName, 'wb')  # this will overwrite current contents
        cPickle.dump(self.W.get_value(borrow=True), save_file, -1)  
        cPickle.dump(self.vbias.get_value(borrow=True), save_file, -1)
        cPickle.dump(self.hbias.get_value(borrow=True), save_file, -1)
        save_file.close()

    def restore_from_cache(self, path=CACHE_PATH):
        fileName = "rbm_rs_%s_%s.model" % (self.num_vis, self.num_hid)
        fileName = os.path.join(path, fileName)
        if os.path.isfile(fileName):
            fileName_p = open(fileName, 'r')
            self.W.set_value(cPickle.load(fileName_p), borrow=True)
            self.vbias.set_value(cPickle.load(fileName_p), borrow=True)
            self.hbias.set_value(cPickle.load(fileName_p), borrow=True)
            fileName_p.close()
            self.need_train = False
        else:
            print "Model file was not found. Need to call RBM.save_model()"
    
    def init_W(self):
        initial_W = np.asarray(0.0001 * self.numpy_rng.randn(self.num_vis, self.num_hid), dtype=theano.config.floatX)
        self.W = theano.shared(value=initial_W, name='W', borrow=True)
        self.W_inc = theano.shared(value=np.zeros((self.num_vis, self.num_hid), dtype=theano.config.floatX), name='W_inc', borrow=True)
    def init_hbias(self):
        self.hbias = theano.shared(value=np.zeros(self.num_hid, dtype=theano.config.floatX), name='hbias', borrow=True)
        self.hbias_inc = theano.shared(value=np.zeros(self.num_hid, dtype=theano.config.floatX), name='hbias_inc', borrow=True)
    def init_vbias(self):      
        self.vbias = theano.shared(value=np.zeros(self.num_vis, dtype=theano.config.floatX), name='vbias', borrow=True)
        self.vbias_inc = theano.shared(value=np.zeros(self.num_vis, dtype=theano.config.floatX), name='vbias_inc', borrow=True)    
    
    def init_params(self):
        self.init_W()
        self.init_vbias()
        self.init_hbias()

    def prop_up(self, vis):
        pre_sigmoid_activation = T.dot(vis, self.W) + T.outer(self.D,self.hbias)
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]

    def prop_down(self, hid):
        pre_softmax_activation = T.dot(hid, self.W.T) + self.vbias
        return [pre_softmax_activation, T.nnet.softmax(pre_softmax_activation)]

    def free_energy(self, v_sample):
        wx_b = T.dot(v_sample, self.W) + T.outer(self.D, self.hbias)
        vbias_term = T.dot(v_sample, self.vbias)
        hidden_term = T.sum(T.log(1 + T.exp(wx_b)), axis=1)
        return -hidden_term - vbias_term

    def sample_v_given_h(self, h_sample):
        pre_softmax_v, v_mean = self.prop_down(h_sample)
        v_sample = self.theano_rng.multinomial(n=self.D, pvals=v_mean, dtype=theano.config.floatX)
        return [pre_softmax_v, v_mean, v_sample]

    def sample_h_given_v(self, v_sample):
        pre_sigmoid_h, h_mean = self.prop_up(v_sample)
        h_sample = self.theano_rng.binomial(size=h_mean.shape, n=1, p=h_mean, dtype=theano.config.floatX)
        return [pre_sigmoid_h, h_mean, h_sample]

    def gibbs_hvh(self, h0_sample):
        pre_softmax_v1, v1_mean, v1_sample = self.sample_v_given_h(h0_sample)
        pre_sigmoid_h1, h1_mean, h1_sample = self.sample_h_given_v(v1_sample)
        return [pre_softmax_v1, v1_mean, v1_sample,
                pre_sigmoid_h1, h1_mean, h1_sample]

    def gibbs_vhv(self, v0_sample):
        pre_sigmoid_h1, h1_mean, h1_sample = self.sample_h_given_v(v0_sample)
        pre_softmax_v1, v1_mean, v1_sample = self.sample_v_given_h(h1_sample)
        return [pre_sigmoid_h1, h1_mean, h1_sample,
                pre_softmax_v1, v1_mean, v1_sample]

    def get_cost_updates(self, train_params):
        l_rate = T.cast(train_params['learning_rate'], dtype=theano.config.floatX)
        weight_decay = T.cast(train_params['weight_decay'], dtype=theano.config.floatX)
        momentum = T.cast(train_params['momentum'], dtype=theano.config.floatX)
        init_momentum = T.cast(train_params['init_momentum'], dtype=theano.config.floatX)
        moment_start = train_params['moment_start']
        
        batch_size = T.cast(train_params['batch_size'], dtype=theano.config.floatX)
        cd_steps = train_params['cd_steps']
        persistent = train_params['persistent']
        batch_size = T.cast(train_params['batch_size'], dtype=theano.config.floatX)

        # compute positive phase
        pre_sigmoid_ph, ph_mean, ph_sample = self.sample_h_given_v(self.input)

        if persistent is None:
            chain_start = ph_sample
        else:
            chain_start = persistent
        
        [pre_softmax_nvs, nv_means, nv_samples,
         pre_sigmoid_nhs, nh_means, nh_samples], updates = \
            theano.scan(self.gibbs_hvh,
                    outputs_info=[None,  None,  None, None, None, chain_start],
                    n_steps=cd_steps)

        vis_probs_fant = nv_samples[-1]
        hid_probs_fant = nh_means[-1]
        
        cur_momentum = T.switch(T.lt(self.epoch_ratio[0], moment_start), init_momentum, momentum)
        # updates
        
        W_inc = ( T.dot(self.input.T, ph_mean) - T.dot(vis_probs_fant.T, hid_probs_fant) )/batch_size - self.W * weight_decay
        hbias_inc = (T.sum(ph_mean, axis=0) - T.sum(hid_probs_fant,axis=0))/batch_size
        vbias_inc = (T.sum(self.input,axis=0) - T.sum(vis_probs_fant,axis=0))/batch_size
        
        W_inc_rate = (self.W_inc * cur_momentum + W_inc) * l_rate
        updates[self.W] = self.W + W_inc_rate
        updates[self.hbias] = self.hbias + (self.hbias_inc * cur_momentum + hbias_inc) * l_rate
        updates[self.vbias] = self.vbias + (self.vbias_inc * cur_momentum + vbias_inc) * l_rate/self.D
        updates[self.W_inc] = W_inc
        updates[self.hbias_inc] = hbias_inc
        updates[self.vbias_inc] = vbias_inc

        current_free_energy = T.mean(self.free_energy(self.input))
        
        
        if persistent:
            # Note that this works only if persistent is a shared variable
            updates[persistent] = nh_samples[-1]
            # pseudo-likelihood is a better proxy for PCD
            monitoring_cost = self.get_pseudo_likelihood_cost(updates)
        else:
            # reconstruction cross-entropy is a better proxy for CD
            monitoring_cost = self.get_reconstruction_cost(updates,
                                                           pre_softmax_nvs[-1])

        return monitoring_cost, current_free_energy, T.mean(W_inc_rate), updates

    def get_pseudo_likelihood_cost(self, updates):
        """Stochastic approximation to the pseudo-likelihood"""

        bit_i_idx = theano.shared(value=0, name='bit_i_idx')

        xi = T.round(self.input)

        fe_xi = self.free_energy(xi)
        xi_flip = T.set_subtensor(xi[:, bit_i_idx], 1 - xi[:, bit_i_idx])

        fe_xi_flip = self.free_energy(xi_flip)

        cost = T.mean(self.num_vis * T.log(T.nnet.sigmoid(fe_xi_flip -
                                                            fe_xi)))

        updates[bit_i_idx] = (bit_i_idx + 1) % self.num_vis

        return cost

    def get_reconstruction_cost(self, vis_sample):
        return T.sum(T.sum(T.sqr(self.input - vis_sample), axis=1))/self.D

