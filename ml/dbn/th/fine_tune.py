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


def get_error_step():
    pre_h, h_mean = r.prop_up(x)
    pre_v, v_mean = r.prop_down(h_mean)
    error = T.sum(T.sum( (x - v_mean) ** 2 ))
    return error


def get_error(data,batch_size=70):
    error = get_error_step()
    get_batch_error = theano.function([index], error, givens=[(x, data_sh[index * batch_size: (index + 1) * batch_size])])
    err = []
    for i in xrange(0, batch_size):
        err.append(get_batch_error(i))
    return err


def fine_tune_step(rbm, learning_rate=0.1):
    hid = T.nnet.sigmoid(T.dot(x, rbm.W) + r.hbias)
    vis = T.nnet.sigmoid(T.dot(hid, rbm.W.T) + r.vbias)
    L = - T.sum(x * T.log(vis) + (1 - x) * T.log(1 - vis), axis=1)
    cost = T.mean(L)
    gparams = T.grad(cost, rbm.params)
    updates = []
    for param, gparam in zip(rbm.params, gparams):
        updates.append((param, param - learning_rate * gparam))
    
    return cost, updates

def fine_tune(rbm, data, epochs = 20, learning_rate=0.1, batch_size=70)
    cost, updates = fine_tune_step(rbm, learning_rate=0.1)    
    fine_tune = theano.function([index], cost, updates=updates, givens=[(x, data[index * batch_size: (index + 1) * batch_size])])
    for ep in xrange(0, epochs):
        sum_cost = 0
        for i in xrange(0, num_batches):
            sum_cost += fine_tune(i)
        print "Epoch # %d sum_cost: %f" % (ep, sum_cost)
    return rbm
