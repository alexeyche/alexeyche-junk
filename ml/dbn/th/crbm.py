#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np
import os
from theano.tensor.shared_randomstreams import RandomStreams
import cPickle

CACHE_PATH="/mnt/yandex.disk/models/temp"

class CRBM(object):
    def __init__(self, input = None, history = None, num_vis = 10, num_hid = 15, n_delay = 3, from_cache=True):
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
        initial_W_uv = np.asarray(0.01 * self.numpy_rng.randn(self.n_delay, self.num_vis, self.num_vis), dtype=theano.config.floatX)
        self.W_uv = theano.shared(value=initial_W_uv, name='W_uv', borrow=True)
        initial_W_uh = np.asarray(0.01 * self.numpy_rng.randn(self.n_delay, self.num_vis, self.num_hid), dtype=theano.config.floatX)
        self.W_uh = theano.shared(value=initial_W_uh, name='W_uh', borrow=True)
       
        self.hbias = theano.shared(value=np.zeros(self.num_hid, dtype=theano.config.floatX), name='hbias', borrow=True)
        self.vbias = theano.shared(value=np.zeros(self.num_vis, dtype=theano.config.floatX), name='vbias', borrow=True)
        (self.vbias_c, self.hbias_c) = self.calc_bias_c()
        self.need_train = True
        if from_cache == True:
            self.restore_from_cache()
        # some useful service vars
        self.W_inc = theano.shared(value=np.zeros((self.num_vis,self.num_hid), dtype=theano.config.floatX), name='W_inc', borrow=True)
        self.vbias_inc = theano.shared(value=np.zeros(self.num_vis, dtype=theano.config.floatX), name='hbias_inc', borrow=True)
        self.hbias_inc = theano.shared(value=np.zeros(self.num_hid, dtype=theano.config.floatX), name='vbias_inc', borrow=True)
        self.W_uv_inc = theano.shared(value=np.zeros((self.n_delay, self.num_vis,self.num_vis), dtype=theano.config.floatX), name='W_uh_inc', borrow=True)
        self.W_uh_inc = theano.shared(value=np.zeros((self.n_delay, self.num_vis,self.num_hid), dtype=theano.config.floatX), name='W_uv_inc', borrow=True)
        
        self.epoch_ratio = theano.shared(np.zeros((1), dtype=theano.config.floatX), borrow=True)
        
    def save_model(self):
        fileName = "crbm_%s_%s_%s.model" % (self.num_vis, self.num_hid, self.n_delay)
        fileName = os.path.join(CACHE_PATH, fileName)
        save_file = open(fileName, 'wb')  # this will overwrite current contents
        cPickle.dump(self.W.get_value(borrow=True), save_file, -1)  
        cPickle.dump(self.W_uv.get_value(borrow=True), save_file, -1)  
        cPickle.dump(self.W_uh.get_value(borrow=True), save_file, -1) 
        cPickle.dump(self.vbias.get_value(borrow=True), save_file, -1)
        cPickle.dump(self.hbias.get_value(borrow=True), save_file, -1)
        save_file.close()

    def restore_from_cache(self):
        fileName = "crbm_%s_%s_%s.model" % (self.num_vis, self.num_hid, self.n_delay)
        fileName = os.path.join(CACHE_PATH, fileName)
        if os.path.isfile(fileName):
            fileName_p = open(fileName, 'r')
            self.W.set_value(cPickle.load(fileName_p), borrow=True)
            self.W_uv.set_value(cPickle.load(fileName_p), borrow=True)
            self.W_uh.set_value(cPickle.load(fileName_p), borrow=True)
            self.vbias.set_value(cPickle.load(fileName_p), borrow=True)
            self.hbias.set_value(cPickle.load(fileName_p), borrow=True)
            fileName_p.close()
            self.need_train = False
        else:
            print "Model file was not found. Need to call CRBM.save_model()"

    def calc_bias_c(self):
        vbias_c = hbias_c = None
        for t in xrange(0, self.n_delay):
            vbias_c_t = T.dot(self.history[t,:,:], self.W_uv[t,:,:])                    
            hbias_c_t = T.dot(self.history[t,:,:], self.W_uh[t,:,:])                    
            if vbias_c:
                vbias_c += vbias_c_t
            else:
                vbias_c = vbias_c_t
            if hbias_c:
                hbias_c += hbias_c_t
            else:
                hbias_c = hbias_c_t
        return (vbias_c, hbias_c) 
 
    def prop_up(self, vis):
        pre_sigmoid_activation = T.dot(vis, self.W) + self.hbias + self.hbias_c
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]

    def prop_down(self, hid):
        pre_sigmoid_activation = T.dot(hid, self.W.T) + self.vbias + self.vbias_c
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]
    
   
    def free_energy(self, v_sample):
        wx_b = T.dot(v_sample, self.W) + self.hbias + self.hbias_c
        biased_sample = v_sample + self.vbias_c
        vbias_term = T.dot(biased_sample, self.vbias)  
        hidden_term = T.sum(T.log(1 + T.exp(wx_b)), axis=1)
        return -hidden_term - vbias_term
       
    def sample_v_given_h(self, h_sample):
        pre_sigmoid_v, v_mean = self.prop_down(h_sample)
        v_sample = self.theano_rng.binomial(size=v_mean.shape, n=1, p=v_mean, dtype=theano.config.floatX)
        return [pre_sigmoid_v, v_mean, v_sample]

    def sample_h_given_v(self, v_sample):
        pre_sigmoid_h, h_mean = self.prop_up(v_sample)
        h_sample = self.theano_rng.binomial(size=h_mean.shape, n=1, p=h_mean, dtype=theano.config.floatX)
        return [pre_sigmoid_h, h_mean, h_sample]

    def gibbs_hvh(self, h0_sample):
        pre_sigmoid_v1, v1_mean, v1_sample = self.sample_v_given_h(h0_sample)
        pre_sigmoid_h1, h1_mean, h1_sample = self.sample_h_given_v(v1_sample)
        return [pre_sigmoid_v1, v1_mean, v1_sample,
                pre_sigmoid_h1, h1_mean, h1_sample]

    def gibbs_vhv(self, v0_sample):
        pre_sigmoid_h1, h1_mean, h1_sample = self.sample_h_given_v(v0_sample)
        pre_sigmoid_v1, v1_mean, v1_sample = self.sample_v_given_h(h1_sample)
        return [pre_sigmoid_h1, h1_mean, h1_sample,
                pre_sigmoid_v1, v1_mean, v1_sample]
    
    def get_reconstruction_cost(self, updates, vis_probs):
        return T.sum(T.sum(T.sqr(self.input - vis_probs), axis=1))
            
    def get_cost_updates(self, train_params):
        l_rate = T.cast(train_params['learning_rate'], dtype=theano.config.floatX)
        weight_decay = T.cast(train_params['weight_decay'], dtype=theano.config.floatX)
        momentum = T.cast(train_params['momentum'], dtype=theano.config.floatX)
        init_momentum = T.cast(train_params['init_momentum'], dtype=theano.config.floatX)
        moment_start = train_params['moment_start']
        batch_size = T.cast(train_params['batch_size'], dtype=theano.config.floatX)
        cd_steps =  train_params['cd_steps']
        
        # compute positive phase
        pre_sigmoid_ph, ph_mean, ph_sample = self.sample_h_given_v(self.input)

        hid_states = ph_sample

        [pre_sigmoid_nvs, nv_means, nv_samples,
         pre_sigmoid_nhs, nh_means, nh_samples], updates = \
            theano.scan(self.gibbs_hvh,
                    outputs_info=[None,  None,  None, None, None, hid_states],
                    n_steps=cd_steps)

        vis_probs_fant = nv_means[-1]
        hid_probs_fant = nh_means[-1]

        # updates
        cur_momentum = T.switch(T.lt(self.epoch_ratio[0], moment_start), init_momentum, momentum)
        
        W_inc = cur_momentum * self.W_inc + l_rate*((T.dot(self.input.T, hid_states) - T.dot(vis_probs_fant.T, hid_probs_fant))/batch_size - self.W * weight_decay)
        vbias_inc = cur_momentum * self.vbias_inc + l_rate*((T.sum( self.input - self.vbias - self.vbias_c ,axis=0) -
                                                                  T.sum( vis_probs_fant - self.vbias - self.vbias_c, axis=0))/batch_size)
        hbias_inc = cur_momentum * self.hbias_inc + l_rate*((T.sum(hid_states, axis=0) -
                                                                  T.sum(hid_probs_fant, axis=0))/batch_size)
        updates[self.W] = self.W + W_inc 
        updates[self.vbias] = self.vbias + vbias_inc
        updates[self.hbias] = self.hbias + hbias_inc
        updates[self.W_inc] = W_inc
        updates[self.vbias_inc] = vbias_inc
        updates[self.hbias_inc] = hbias_inc

        W_uv_temp = [] 
        W_uh_temp = [] 
        for t in xrange(0, self.n_delay):
            W_uv_temp.append( self.W_uv_inc[t,:,:] * cur_momentum + \
                                                l_rate*((T.dot(self.history[t,:,:].T, self.input - self.vbias - self.vbias_c) - \
                                                         T.dot(self.history[t,:,:].T, vis_probs_fant - self.vbias - self.vbias_c))/batch_size) )
            W_uh_temp.append( self.W_uh_inc[t,:,:] * cur_momentum + \
                                                l_rate*((T.dot(self.history[t,:,:].T, hid_states) - \
                                                         T.dot(self.history[t,:,:].T, hid_probs_fant))/batch_size) )
        W_uv_inc = T.stack(W_uv_temp)[0]
        W_uh_inc = T.stack(W_uh_temp)[0]

        updates[self.W_uv] = self.W_uv + W_uv_inc
        updates[self.W_uh] = self.W_uh + W_uh_inc
        updates[self.W_uv_inc] = W_uv_inc
        updates[self.W_uh_inc] = W_uh_inc


        # various cost calculations
        current_free_energy = T.mean(self.free_energy(self.input))
        energy_cost = current_free_energy - T.mean(self.free_energy(vis_probs_fant))

        recon_cost = self.get_reconstruction_cost(updates, vis_probs_fant)
        
        return recon_cost, current_free_energy, energy_cost, updates
               

class CRBMSoftmax(CRBM):
    def prop_down(self, hid):
        pre_softmax_activation = T.dot(hid, self.W.T) + self.vbias + self.vbias_c
        return [pre_softmax_activation, T.nnet.softmax(pre_softmax_activation)]


def generate(crbm, history, sample_n = 100, gibbs_steps = 30):
    samples = list()
    samples_prob = list()

    numpy_rng = np.random.RandomState(1)
    vis = theano.shared(np.asarray(np.abs(0.01 * numpy_rng.randn(1, crbm.num_vis)), dtype=theano.config.floatX))
    history_sh = theano.shared(np.asarray(history, dtype=theano.config.floatX))
    [pre_sigmoid_h1, h1_mean, h1_sample,
                pre_sigmoid_v1, v1_mean, v1_sample], updates = theano.scan(crbm.gibbs_vhv, 
                                outputs_info = [None,None,None,None,None,vis], 
                                n_steps=gibbs_steps)

    updates.update( [( vis , v1_sample[-1]) , (history_sh, T.concatenate( (v1_sample[-1].dimshuffle('x',0,1), history_sh[:-1,:,:]))) ] )
    dream = theano.function([], [v1_mean[-1], v1_sample[-1]], updates = updates, givens = [(crbm.input, vis),(crbm.history, history_sh)])            

    for s in xrange(0, sample_n):
        vis, vis_s = dream()
        samples.append(vis_s[0])
        samples_prob.append(vis[0])
    return samples, samples_prob

