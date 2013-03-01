#!/usr/bin/PScript
import numpy as np
import theano
import theano.tensor as T
import rpy2.robjects.lib.ggplot2 as ggplot2
import math, datetime
import rpy2.robjects as ro
from rpy2.robjects.packages import importr
from rpy2.robjects.vectors import IntVector, FloatVector

def gray_plot(data):
    reshape = importr('reshape')
    gg = ggplot2.ggplot(reshape.melt(data,id_var=['x','y']))
    pg = gg + ggplot2.aes_string(x='L1',y='L2')+ \
         ggplot2.geom_tile(ggplot2.aes_string(fill='value'))+ \
         ggplot2.scale_fill_gradient(low="black", high="white",limits=IntVector((0,1)))+ \
         ggplot2.coord_equal()
    pg.plot() 

def daydream(rbm):
    steps = 100
    numpy_rng = np.random.RandomState(1)
    init_vis = theano.shared(np.asarray(0.01 * numpy_rng.randn(10, rbm.num_vis), dtype=theano.config.floatX))
    [pre_sigmoid_h1, h1_mean, h1_sample,
                pre_sigmoid_v1, v1_mean, v1_sample], updates = theano.scan(rbm.gibbs_vhv, outputs_info = [None,None,None,None,None,init_vis], n_steps=steps)
    f = theano.function([], v1_mean[-1], updates = updates)            
    v = f()
    gray_plot(v.tolist())

def gen_name(rbms,params):
    name = "rbms"
    for rbm in rbms.stack:
        name += "_%d" % (rbm.num_hid) 
    name += "_%s_%s_%s" % (params['learning_rate'], params['cd_steps'], params['persistent'])
    return name        
   
