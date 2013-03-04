#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os, sys
from theano.tensor.shared_randomstreams import RandomStreams
from theano import ProfileMode
import cPickle

from rbm import RBMReplSoftmax

x = [[0,1,1,20,3],[1,1,2,0,1],[0,1,1,1,1]]
#x_np = np.asarray(x, dtype=theano.config.floatX)
x_sh = theano.shared(np.asarray(x, dtype=theano.config.floatX), borrow=True) 

rbm = RBMReplSoftmax(num_vis = 5, num_hid = 10)

pre_val, val = rbm.prop_up(rbm.input)
pre_val2, val2 = rbm.prop_down(val)
#size = [D1,D2,D3] for each case
#pvals = [0.2,0.2,0.2,0.2,0.2]  for each visible unit

v_sample = rbm.theano_rng.multinomial(size=val2.shape, n=1, pvals=val2, dtype=theano.config.floatX)

f = theano.function([], [pre_val2, val2], givens=[(rbm.input, x_sh)])
