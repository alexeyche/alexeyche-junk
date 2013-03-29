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

from rbm_rs import RBMReplSoftmax
from rbm_stack import RBMStack
from rbm_util import gray_plot
from rpy2.robjects.packages import importr

def getch():
    import sys, tty, termios
    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        return sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)

csvfile = "/home/alexeyche/prog/topic/nips_feats.csv"
#csvfile = "/home/alexeyche/my/git/alexeyche-junk/ml/dbn/sentiment/training_feat.csv"
data = np.asarray(genfromtxt(csvfile, delimiter=','), dtype=theano.config.floatX)
data = np.round(np.log(data[:,0:2000]+1))
#data = data[:,0:2000]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

perm = np.random.permutation(num_cases)
data = data[perm]
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)

train_params = {'batch_size' : 50, 'learning_rate' : 0.01, 'cd_steps' : 1, 'max_epoch' : 1000, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }

rbm = RBMReplSoftmax(num_vis = num_vis, num_hid = 200, from_cache = True)

num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']
max_epoch = train_params['max_epoch']
train_params['ep_inc'] = np.float32(1.0/(num_batches*max_epoch))
ep_inc = train_params['ep_inc']
persistent = train_params['persistent_on']
batch_size = train_params['batch_size']
index = T.lscalar('index')  # index to a minibatch

if persistent:
    train_params['persistent'] = theano.shared(np.zeros((batch_size, rbm.num_hid), dtype=theano.config.floatX), borrow=True)
else:
    train_params['persistent'] = None
cost, free_en, gparam, updates, w = rbm.get_cost_updates(train_params)
updates.update([(rbm.epoch_ratio, rbm.epoch_ratio + ep_inc)])

f = theano.function([index], [cost, free_en, gparam] + w,
       updates=updates,
       givens=[(rbm.input, data_sh[index * batch_size: (index + 1) * batch_size])])
i=0
def print_watches(w):
    global i
    print "pos hid_m %s" % w[0][0:3,0:12]
    grdevices = importr('grDevices')
    grdevices.png(file="/home/alexeyche/tmp/hids%d.png" % i, width=1024, height=1024)
    i+=1
    gray_plot(w[0].tolist(), w[0].max(), w[0].min())
    grdevices.dev_off()
    print "pos hid_s %s" % w[1][0:3,0:12]
    print "neg vis_m %s" % w[2][0:3,0:12]
    print "neg vis_s %s" % w[3][0:3,0:12]
    print "neg hid_m %s" % w[4][0:3,0:12]
    print "W_inc %s" % w[5][0:3,0:12]
    print "hbias_inc %s" % w[6][0:12]
    print "vbias_inc %s" % w[7][0:12]
    print "free en %s" % w[8][0:12]
#    import pdb; pdb.set_trace()

def train_rs(rbm, train_params):
    max_epoch = train_params['max_epoch']
    num_batches = data_sh.get_value(borrow=True).shape[0]/train_params['batch_size']

    mean_cost = []
    mean_cost_last = 0
    for ep in xrange(0, max_epoch):
        for b in xrange(0, num_batches):
            out = f(b)
            cost, cur_free_en, cur_gparam = out[0:3]
            w = out[3:]
            
            mean_cost.append(cost)
            epoch_ratio = rbm.epoch_ratio.get_value(borrow=True)
            print "pretrain(%3.1f), layer %s, epoch # %d:%d last mean cost %2.2f (cost: %f) free energy: %f grad: %f" % (epoch_ratio*100,0, ep, b, mean_cost_last, cost, cur_free_en, cur_gparam)
        mean_cost_last = np.mean(mean_cost)
        mean_cost = []
        if ep % 25 == 0:
            print_watches(w)

if rbm.need_train:
    train_params = {'batch_size' : 87, 'learning_rate' : 0.01, 'cd_steps' : 1, 'max_epoch' : 200, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }


    train_rs(rbm, train_params)

    train_params = {'batch_size' : 87, 'learning_rate' : 0.01, 'cd_steps' : 3, 'max_epoch' : 300, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }

    train_rs(rbm, train_params)

    train_params = {'batch_size' : 87, 'learning_rate' : 0.01, 'cd_steps' : 5, 'max_epoch' : 400, 'persistent_on' : False, 'init_momentum' : 0.5, 'momentum' : 0.9, 'moment_start' : 0.01, 'weight_decay' : 0.0001 }

    train_rs(rbm, train_params)
    rbm.save_model()

preh,h = rbm.prop_up(data[0:2])
prev,v, vs = rbm.sample_v_given_h(h)

f = theano.function([], [v, vs], givens=[(rbm.input,data[0:2])])
