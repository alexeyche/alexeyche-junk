#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np


def kernel_epanechnikov(x):
    return T.switch(T.gt(T.abs_(x),1), 0, 0.75 * (1-T.sqr(x)))

numpy_rng = np.random.RandomState(2)
data = np.asarray(numpy_rng.randn(50*5000), dtype=theano.config.floatX)

x = T.fvector()
h, e = np.histogram(np.abs(np.diff(data)))
freq_diff = e[h.argmax()]
freq_f = h[h.argmax()]
h2 = h[h != h[h.argmax()]]
e2 = e[e != freq_diff]
next_freq_diff = e2[h2.argmax()]
next_freq_f = h2[h2.argmax()]

h = np.abs((freq_f*freq_diff-next_freq_f*next_freq_diff)/(freq_f+next_freq_f))
dens, updates = theano.scan(lambda x_data, x: kernel_epanechnikov(T.abs_((x-x_data)/h)), sequences = data, non_sequences = x)
f = T.sum(dens, axis=0)/(h*data.shape[0])
fun = theano.function([x], f, updates=updates)

#p = fun(data)
