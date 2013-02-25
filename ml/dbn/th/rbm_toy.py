#!/usr/bin/PScript
import theano
import theano.tensor as T
from util import getch
import numpy as np
from numpy import genfromtxt
import os
from theano.tensor.shared_randomstreams import RandomStreams
from theano import ProfileMode
################
from rbm_util import daydream
from rbm import RBM

num_vis = 50
num_hid = 10
num_dims = num_vis
num_cases = 1000
#csvfile = os.popen("./gen_data.R two_binom %d %d" % (num_cases, num_dims,)).read()
csvfile = "two_binom.csv"
data = genfromtxt(csvfile, delimiter=',')


data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)

x = T.matrix('x')  # the data is presented as rasterized images
index = T.lscalar()    # index to a [mini]batch
batch_size = 100

r = RBM(input = x, num_vis = num_vis, num_hid = num_hid)
#f = theano.function([index], 
#             r.prop_up(x), 
#             givens = [( x, data_sh[ index*batch_size: (index+1)*batch_size] )] 
#             )

#chain_start = r.sample_h_given_v(r.input)
#[pre_sigmoid_nvs, nv_means, nv_samples,
# pre_sigmoid_nhs, nh_means, nh_samples], updates = \
#    theano.scan(r.gibbs_hvh,
#            outputs_info=[None,  None,  None, None, None, chain_start],
#            n_steps=10)
#f = theano.function([],nh_samples[-1], updates = updates, givens =  [( x, data_sh[ 0*batch_size: (0+1)*batch_size] )])
cost, updates = r.get_cost_updates(lr=0.1,
                                         persistent=None, k=15)
train_rbm = theano.function([index], cost,
           updates=updates,
           givens=[(x, data_sh[index * batch_size: (index + 1) * batch_size])],
           name='train_rbm')

max_epoch = 100
for ep in xrange(0,max_epoch):
    for i in xrange(0,10):
        print train_rbm(i)







