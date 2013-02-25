#!/usr/bin/PScript
import theano
import theano.tensor as T
from theano import function
from util import getch
import numpy
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams

numpy_rng = numpy.random.RandomState(1)
W_values = numpy.asarray(0.01 * numpy_rng.randn(50, 10), dtype=theano.config.floatX)

W = theano.shared ( W_values ) # we assume that ``W_values`` contains the
                               # initial values of your weight matrix

bhid = theano.shared(value=numpy.zeros(10,
                    dtype=theano.config.floatX),
                    name='hbias', borrow=True)
bvis = theano.shared(value=numpy.zeros(50,
                    dtype=theano.config.floatX),
                    name='hbias', borrow=True)
trng = T.shared_randomstreams.RandomStreams(1234)

def OneStep( vsample) :
   hmean   = T.nnet.sigmoid( theano.dot( vsample, W) + bhid)
   hsample = trng.binomial( size = hmean.shape, n = 1, prob = hmean, )
   vmean   = T.nnet.sigmoid( theano.dot( hsample, W.T) + bvis)
   return trng.binomial( size = vsample.shape, n = 1, prob = vsample, dtype=theano.config.floatX)

sample = theano.tensor.vector()

csvfile = os.popen("./gen_data.R two_binom %d %d" % (10, 5,)).read()
data = genfromtxt(csvfile, delimiter=',')
data_sh = theano.shared(numpy.asarray(data, dtype=theano.config.floatX), borrow=True)

s = T.sum(data_sh, axis=1) - T.sum(data_sh, axis=1) * T.cast(2.0, dtype=theano.config.floatX)
f = theano.function([],s)
