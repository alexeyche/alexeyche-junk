#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np

d1 = theano.shared(np.asarray([[[1,2,3]],[[1,2,3]],[[1,2,3]]], dtype = theano.config.floatX), borrow=True)
d2 = theano.shared(np.asarray([[4,5,6]], dtype = theano.config.floatX), borrow=True)

#l = [d1, d2, d3]
#ans = T.concatenate(l).reshape((3,2,3))
#ans = T.stack(l)[0]
ans = T.concatenate( (d2.dimshuffle('x',0,1), d1))

f = theano.function([], ans)


