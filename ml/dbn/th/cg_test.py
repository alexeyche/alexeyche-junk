#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np


# simple autoencoder


num_vis = 20
num_hid = 10

num_cases = 1000

numpy_rng = np.random.RandomState(1)

d = theano.shared(np.asarray(numpy_rng.randn(num_cases, num_vis), dtype=theano.config.floatX), borrow=True)



theta_sh = theano.shared( np.zeros(num_vis*num_hid, dtype=theano.config.floatX), borrow=True)


def cook_params(theta):
    W = theta[0:num_vis*num_hid].reshape((num_vis,num_hid))
#    vb = theta[num_vis*num_hid:num_vis*num_hid+num_vis]
#    hb = theta[num_vis*num_hid+num_vis:num_vis*num_hid+num_vis+num_hid]
    return W #, vb, hb


def evaluate(W, vb, hb):
    h = T.nnet.sigmoid( T.dot(d, W) + hb ) 
    v = T.nnet.sigmoid( T.dot(h, W.T) + vb ) 
    return h,v 

#W, vb, hb = cook_params(theta_sh)
W = cook_params(theta_sh)

h, v = evaluate(W) #, vb, hb)
cost = T.mean(T.sum( T.sqr(v-d), axis=1 ))


def fun(theta):
    theta_sh.set_value(theta, borrow=True)
    get_cost = theano.function([], cost)
    return get_cost()

# own try:
def grad_fun(theta)
    theta_sh.set_value(theta, borrow=True)

    IO= 1/num_cases * (v-d)  
    ix2=IO
    dw2 = T.dot(h.T, ix2) 
    ix1=(T.dot(ix2, W)+hb) * h * (1-h)
    dw1=T.dot(v.T, ix1) 
    return dw1, dw2

#def grad_fun(theta):
#    theta_sh.set_value(theta, borrow=True)
#    grad = T.grad(cost, theta_sh)
#    get_grad = theano.function([], grad)
#    return get_grad()

def callback(theta):
    theta_sh.set_value(theta, borrow=True)
    print "cost is: ", fun(theta)

import scipy.optimize
import time

type = 'tnc'

if type == 'cg':
    print ("Optimizing using scipy.optimize.fmin_cg...")
    start_time = time.clock()
    best_w_b = scipy.optimize.fmin_cg(
               f=fun,
               x0=np.zeros(num_vis*num_hid+num_vis+num_hid, dtype=theano.config.floatX),
               fprime=grad_fun,
               callback=callback,
               disp=0,
               maxiter=100)
    end_time = time.clock()
if type == 'tnc':
    print ("Optimizing using scipy.optimize.fmin_tnc...")
    start_time = time.clock()
    best_w_b = scipy.optimize.fmin_tnc(
               func=fun,
               x0=np.zeros(num_vis*num_hid+num_vis+num_hid, dtype=theano.config.floatX),
               fprime=grad_fun,
#               callback=callback,
               disp=5)
 #              maxiter=100)
    end_time = time.clock()



#g = theano.function([], grad_fun(theta_sh))()
