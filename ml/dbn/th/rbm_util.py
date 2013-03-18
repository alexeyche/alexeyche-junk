#!/usr/bin/PScript
import numpy as np
import theano
import theano.tensor as T
import rpy2.robjects.lib.ggplot2 as ggplot2
import math, datetime
import rpy2.robjects as ro
from rpy2.robjects.packages import importr
from rpy2.robjects.vectors import IntVector, FloatVector
import os
import cPickle

def gray_plot(data, min=0, max=1):
    reshape = importr('reshape')
    gg = ggplot2.ggplot(reshape.melt(data,id_var=['x','y']))
    pg = gg + ggplot2.aes_string(x='L1',y='L2')+ \
         ggplot2.geom_tile(ggplot2.aes_string(fill='value'))+ \
         ggplot2.scale_fill_gradient(low="black", high="white",limits=IntVector((min,max)))+ \
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

path="/mnt/yandex.disk/models"
def save_to_file(rbms,params):
    name = path+"/"+gen_name(rbms,params)
    fileName_p = open(name, 'wb')
    for rbm in rbms.stack:
        cPickle.dump(rbm.W.get_value(borrow=True), fileName_p, -1)  # the -1 is for HIGHEST_PROTOCOL
        cPickle.dump(rbm.vbias.get_value(borrow=True), fileName_p, -1)  # .. and it triggers much more efficient
        cPickle.dump(rbm.hbias.get_value(borrow=True), fileName_p, -1)  # .. storage than numpy's default
    print "rbm_stack saved %s" % name

def load_from_file(rbms, params):
    name = path+"/"+gen_name(rbms,params)
    if os.path.isfile(name):
        fileName_p = open(name, 'r')
        for rbm in rbms.stack:    
            rbm.W.set_value(cPickle.load(fileName_p), borrow=True)
            rbm.vbias.set_value(cPickle.load(fileName_p), borrow=True)
            rbm.hbias.set_value(cPickle.load(fileName_p), borrow=True)
        rbms.isTrained = True
        print "rbm_stack opened %s" % name
        return True
    else:
        return False
  
def print_top_to_file(ae, train_params, name, data_sh, data_target, cases):
    ae_name = gen_name(ae, train_params)
    fileName = path + "/" + ae_name + "_" + name + ".png"

    out_line = ae.stack[-1].output
    f = theano.function([], out_line, givens=[(ae.input, data_sh[cases])])
    hid_stat = f()
    x = hid_stat[...,0].tolist()
    y = hid_stat[...,1].tolist()
    grdevices = importr('grDevices')
    grdevices.png(file=fileName, width=1024, height=1024)
    lab = ro.IntVector(data_target[cases].tolist())
    lab_col = ro.StrVector(map(lambda p: p == 0 and 'blue' or 'red', lab))
    lab_col.names = lab
    ro.r.plot(x,y, xlab = "x", ylab="y", type="n")
    ro.r.text(x,y,labels=lab, col = lab_col)
    grdevices.dev_off()

