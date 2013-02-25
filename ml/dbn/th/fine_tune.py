#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
from theano import ProfileMode
import cPickle




def get_error(rbm, data, batch_size=70):
    def get_error_step(rbm):
        pre_h, h_mean = rbm.prop_up(x)
        pre_v, v_mean = rbm.prop_down(h_mean)
        error = T.sum(T.sum( (x - v_mean) ** 2 ))
        return error
    
    x = T.matrix('x')  
    index = T.lscalar()    # index to a [mini]batch
    error = get_error_step(rbm)
    get_batch_error = theano.function([index], error, givens=[(x, data[index * batch_size: (index + 1) * batch_size])])
    err = []
    num_batches = data.get_value(borrow=True).shape[0]/batch_size
    for i in xrange(0, num_batches):
        err.append(get_batch_error(i))
    return err



def fine_tune(rbm, data, epochs = 20, learning_rate=0.1, batch_size=70):
    def fine_tune_step(rbm, learning_rate=0.1):
        hid = T.nnet.sigmoid(T.dot(x, rbm.W) + rbm.hbias)
        vis = T.nnet.sigmoid(T.dot(hid, rbm.W.T) + rbm.vbias)
        L = - T.sum(x * T.log(vis) + (1 - x) * T.log(1 - vis), axis=1)
        cost = T.mean(L)
        gparams = T.grad(cost, rbm.params)
        updates = []
        for param, gparam in zip(rbm.params, gparams):
            updates.append((param, param - learning_rate * gparam))
        
        return cost, updates
    
    x = T.matrix('x')  
    index = T.lscalar()    # index to a [mini]batch
    cost, updates = fine_tune_step(rbm, learning_rate=0.1)    
    fine_tune = theano.function([index], cost, updates=updates, givens=[(x, data[index * batch_size: (index + 1) * batch_size])])
    num_batches = data.get_value(borrow=True).shape[0]/batch_size
    for ep in xrange(0, epochs):
        sum_cost = 0
        for i in xrange(0, num_batches):
            sum_cost += fine_tune(i)
        print "Epoch # %d sum_cost: %f" % (ep, sum_cost)
