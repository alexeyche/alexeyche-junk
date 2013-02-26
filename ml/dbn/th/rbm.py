#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
import cPickle

class RBM(object):
    def __init__(self, input=None, num_vis = 50, num_hid = 10, W=None, hbias=None, vbias=None, numpy_rng = None, theano_rng = None):
        if numpy_rng is None:
            numpy_rng = np.random.RandomState(1)
        if theano_rng is None:
            theano_rng = RandomStreams(numpy_rng.randint(2 ** 30))
    
        if W is None:
            initial_W = np.asarray(0.01 * numpy_rng.randn(num_vis, num_hid), dtype=theano.config.floatX)
            W = theano.shared(value=initial_W, name='W', borrow=True)

        if hbias is None:
            # create shared variable for hidden units bias
            hbias = theano.shared(value=np.zeros(num_hid,
                                                    dtype=theano.config.floatX),
                                  name='hbias', borrow=True)
        if vbias is None:
            # create shared variable for visible units bias
            vbias = theano.shared(value=np.zeros(num_vis,
                                                    dtype=theano.config.floatX),
                                  name='vbias', borrow=True)
        # initialize input layer for standalone RBM or layer0 of DBN
        self.input = input
        if input is None:
            self.input = T.matrix('input')

        self.num_vis = num_vis 
        self.num_hid = num_hid
        self.W = W
        self.hbias = hbias
        self.vbias = vbias
        self.theano_rng = theano_rng
        self.params = [self.W, self.hbias, self.vbias]
        _, self.output = self.prop_up(self.input)

    def prop_up(self, vis):
        pre_sigmoid_activation = T.dot(vis, self.W) + self.hbias
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]

    def prop_down(self, hid):
        pre_sigmoid_activation = T.dot(hid, self.W.T) + self.vbias
        return [pre_sigmoid_activation, T.nnet.sigmoid(pre_sigmoid_activation)]

    def free_energy(self, v_sample):
        wx_b = T.dot(v_sample, self.W) + self.hbias
        vbias_term = T.dot(v_sample, self.vbias)
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

    def get_cost_updates(self, lr=0.1, persistent=None, k=1):
        # compute positive phase
        pre_sigmoid_ph, ph_mean, ph_sample = self.sample_h_given_v(self.input)

        if persistent is None:
            chain_start = ph_sample
        else:
            chain_start = persistent

        [pre_sigmoid_nvs, nv_means, nv_samples,
         pre_sigmoid_nhs, nh_means, nh_samples], updates = \
            theano.scan(self.gibbs_hvh,
                    outputs_info=[None,  None,  None, None, None, chain_start],
                    n_steps=k)

        # determine gradients on RBM parameters
        # not that we only need the sample at the end of the chain
        chain_end = nv_samples[-1]
        current_free_energy = T.mean(self.free_energy(self.input))
        cost = current_free_energy - T.mean(
            self.free_energy(chain_end))
        # We must not compute the gradient through the gibbs sampling
        gparams = T.grad(cost, self.params, consider_constant=[chain_end])

        # constructs the update dictionary
        for gparam, param in zip(gparams, self.params):
            # make sure that the learning rate is of the right dtype
            updates[param] = param - gparam * T.cast(lr,
                                                    dtype=theano.config.floatX)
        if persistent:
            # Note that this works only if persistent is a shared variable
            updates[persistent] = nh_samples[-1]
            # pseudo-likelihood is a better proxy for PCD
            monitoring_cost = self.get_pseudo_likelihood_cost(updates)
        else:
            # reconstruction cross-entropy is a better proxy for CD
            monitoring_cost = self.get_reconstruction_cost(updates,
                                                           pre_sigmoid_nvs[-1])

        return monitoring_cost, current_free_energy, T.mean(gparam), updates

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

    def get_reconstruction_cost(self, updates, pre_sigmoid_nv):
        cross_entropy = T.mean(
                T.sum(self.input * T.log(T.nnet.sigmoid(pre_sigmoid_nv)) +
                (1 - self.input) * T.log(1 - T.nnet.sigmoid(pre_sigmoid_nv)),
                      axis=1))
        return cross_entropy


class RBMBinLine(RBM):
    def __init__(self, input=None, num_vis = 50, num_hid = 10, W=None, hbias=None, vbias=None, numpy_rng = None, theano_rng = None):
        if numpy_rng is None:
            numpy_rng = np.random.RandomState(1)
        if theano_rng is None:
            theano_rng = RandomStreams(numpy_rng.randint(2 ** 30))
    
        if W is None:
            initial_W = np.asarray(0.01 * numpy_rng.randn(num_vis, num_hid), dtype=theano.config.floatX)
            W = theano.shared(value=initial_W, name='W', borrow=True)

        if hbias is None:
            # create shared variable for hidden units bias
            hbias = theano.shared(value=np.zeros(num_hid,
                                                    dtype=theano.config.floatX),
                                  name='hbias', borrow=True)
        if vbias is None:
            # create shared variable for visible units bias
            vbias = theano.shared(value=np.zeros(num_vis,
                                                    dtype=theano.config.floatX),
                                  name='vbias', borrow=True)
        self.W_inc = theano.shared(value=np.zeros((num_vis, num_hid),
                                                    dtype=theano.config.floatX),
                                  name='W_inc', borrow=True)
        self.hbias_inc = theano.shared(value=np.zeros(num_hid,
                                                    dtype=theano.config.floatX),
                                  name='hbias_inc', borrow=True)
        self.vbias_inc = theano.shared(value=np.zeros(num_vis,
                                                    dtype=theano.config.floatX),
                                  name='vbias_inc', borrow=True)
        # initialize input layer for standalone RBM or layer0 of DBN
        self.input = input
        if input is None:
            self.input = T.matrix('input')

        self.num_vis = num_vis 
        self.num_hid = num_hid
        self.W = W
        self.hbias = hbias
        self.vbias = vbias
        self.theano_rng = theano_rng
        self.params = [self.W, self.hbias, self.vbias]        
        self.output = self.prop_up(self.input) 
    
    def prop_up(self, vis):
        hid = T.dot(vis, self.W) - self.hbias            
        return hid
    def sample_h_given_v(self, v_sample):
        h_val = self.prop_up(v_sample)
        return h_val
    def free_energy(self):
        return None
    def get_pseudo_likelihood_cost(self):
        return None
    def gibbs_hvh(self, h0_sample):
        pre_sigmoid_v1, v1_mean, v1_sample = self.sample_v_given_h(h0_sample)
        h1_val = self.sample_h_given_v(v1_sample)
        return [pre_sigmoid_v1, v1_mean, v1_sample, h1_val]
    def get_cost_updates(self, lr=0.1, weight_cost = 0.0002, num_cases = 7000, momentum = 0.8, persistent=None, k=1):
        h_val = self.sample_h_given_v(self.input)
        if persistent:
            chain_start = persistent
        else:
            chain_start = h_val

        [pre_sigmoid_nvs, nv_means, nv_samples, nh_vals], updates = \
            theano.scan(self.gibbs_hvh,
                    outputs_info=[None, None, None, chain_start],
                    n_steps=k)

        vis_sample_fantasy = nv_samples[-1]
        hid_val_fantasy = nh_vals[-1]
        momentum_c = T.cast(momentum, dtype=theano.config.floatX)
        num_cases_c = T.cast(num_cases, dtype=theano.config.floatX)
        weight_cost_c = T.cast(weight_cost, dtype=theano.config.floatX)
        lr_c = T.cast(lr, dtype=theano.config.floatX)       

        W_inc = (T.dot(self.input.T, h_val) - T.dot(vis_sample_fantasy.T, hid_val_fantasy))/num_cases_c - self.W * weight_cost_c
        hbias_inc = (T.sum(h_val, axis=0) - T.sum(hid_val_fantasy,axis=0))/num_cases_c
        vbias_inc = (T.sum(self.input,axis=0) - T.sum(vis_sample_fantasy,axis=0))/num_cases_c
        W_inc_rate =  W_inc * momentum_c + self.W_inc * lr_c

        updates[self.W] = self.W +  W_inc_rate
        updates[self.hbias] = self.hbias + self.hbias_inc * momentum_c + hbias_inc * lr_c
        updates[self.vbias] = self.vbias + self.vbias_inc * momentum_c + vbias_inc * lr_c
        updates[self.W_inc] = W_inc
        updates[self.hbias_inc] = hbias_inc
        updates[self.vbias_inc] = vbias_inc

        if persistent:
            updates[persistent] = nh_vals[-1]

        # I don't know for know how to calculate free energy for that kind of rbm units, 
        # so we can measure cost only with reconstruction:
        monitoring_cost = self.get_reconstruction_cost(updates, pre_sigmoid_nvs[-1])

        return monitoring_cost, T.as_tensor_variable(0), T.mean(W_inc_rate), updates




def train_rbm(rbm, data_sh, train_params, saveFile = True, findFile = True):
    num_hid = rbm.num_hid
    if findFile or saveFile:
        fileName = 'rbm_%d' % num_hid

    if findFile and os.path.isfile(fileName):
        fileName_p = open(fileName, 'r')
        rbm.W.set_value(cPickle.load(fileName_p), borrow=True)
        rbm.vbias.set_value(cPickle.load(fileName_p), borrow=True)
        rbm.hbias.set_value(cPickle.load(fileName_p), borrow=True)
        return
    persistent = train_params['persistent']
    batch_size = train_params['batch_size']
    lr_giv = train_params['learning_rate']
    k_giv = train_params['cd_steps']
    max_epoch = train_params['max_epoch']
    num_cases = data_sh.get_value(borrow=True).shape[0]

    num_batches = num_cases/batch_size

    x = T.matrix('x') 
    rbm.input = x
    index = T.lscalar()    # index to a [mini]batch
    if persistent:
        persistent_chain = theano.shared(np.zeros((batch_size, num_hid), dtype=theano.config.floatX), borrow=True)
    else:
        persistent_chain = None

    if type(rbm) is RBMBinLine:
        cost, updates = rbm.get_cost_updates(lr=lr_giv, persistent=persistent_chain, k=k_giv)
        train_rbm_f = theano.function([index], cost,
                   updates=updates,
                   givens=[(x, data_sh[index * batch_size: (index + 1) * batch_size])],
                   name='train_rbm')

        for ep in xrange(0,max_epoch):
            for i in xrange(0,num_batches):
                cost = train_rbm_f(i)
                print "Epoch # %d:%d cost: %f" % (ep, i, cost)

    if type(rbm) is RBM:
        cost, free_en, gparam, updates = rbm.get_cost_updates(lr=lr_giv, persistent=persistent_chain, k=k_giv)

        train_rbm_f = theano.function([index], [cost, free_en, gparam],
                   updates=updates,
                   givens=[(x, data_sh[index * batch_size: (index + 1) * batch_size])],
                   name='train_rbm')

        for ep in xrange(0,max_epoch):
            for i in xrange(0,num_batches):
                cost, cur_free_en, cur_gparam = train_rbm_f(i)
                print "Epoch # %d:%d cost: %f free energy: %f grad: %f" % (ep, i, cost, cur_free_en, cur_gparam)

    if saveFile:
        save_file = open(fileName, 'wb')  # this will overwrite current contents
        cPickle.dump(rbm.W.get_value(borrow=True), save_file, -1)  # the -1 is for HIGHEST_PROTOCOL
        cPickle.dump(rbm.vbias.get_value(borrow=True), save_file, -1)  # .. and it triggers much more efficient
        cPickle.dump(rbm.hbias.get_value(borrow=True), save_file, -1)  # .. storage than numpy's default
        save_file.close()

