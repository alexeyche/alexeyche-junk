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

from ae import AutoEncoder, save_to_file, load_from_file

csvfile = '/home/alexeyche/my/dbn/kaggle_sentiment/training_feat_proc.csv'
#csvfile = '/home/alexeyche/prog/alexeyche-junk/ml/dbn/sentiment/training_feat_proc.csv'
#csvfile = 'two_binom.csv'
data = genfromtxt(csvfile, delimiter=',')
ncol = data.shape[1]
data_target = data[...,0]
data = data[...,1:ncol]
num_cases = data.shape[0]
num_dims = data.shape[1]
num_vis = num_dims

perm = np.random.permutation(num_cases)
data = data[perm]
data_target = data_target[perm]
data_sh = theano.shared(np.asarray(data, dtype=theano.config.floatX), borrow=True)
data_target_sh = theano.shared(np.asarray(data_target, dtype=theano.config.floatX), borrow=True)

batch_size = 100
num_batches = num_cases/batch_size
max_epoch = 20
train_params = {'batch_size' : batch_size, 'learning_rate' : 0.1, 'cd_steps' : 2, 'max_epoch' : max_epoch, 'persistent' : None, 'learning_rate_line' : 0.005 }

sent_ae = AutoEncoder(num_vis = num_vis, hid_layers_size = [500, 2], linear_top=True)
val = sent_ae.finetune_cost()

index = T.lscalar('index')  # index to a minibatch
fun_cost = theano.function([index],val,givens=[(sent_ae.input, data_sh[index * batch_size: (index + 1) * batch_size])])
cost_before = []
for b in xrange(0, num_batches):
        cost_before.append(fun_cost(b))
need_train = False
if need_train:
    fn = sent_ae.pretrain_fun(data_sh, train_params)
    for f in fn:
        for ep in xrange(0, max_epoch):
            for b in xrange(0, num_batches):
                cost, cur_free_en, cur_gparam = f(b)
                print "Epoch # %d:%d cost: %f free energy: %f grad: %f" % (ep, b, cost, cur_free_en, cur_gparam)

    for b in xrange(0, num_batches):
        print "before: %f after: %f" % (cost_before[b], fun_cost(b))

    save_to_file(sent_ae,"ae_936_500_2")
    print "ae saved"
else:
    load_from_file(sent_ae,"ae_936_500_2")

out_line = sent_ae.stack[-1].output
f = theano.function([], out_line, givens=[(sent_ae.input, data[0:100])])
hid_stat = f()
x = hid_stat[...,0].tolist()
y = hid_stat[...,1].tolist()
import rpy2.robjects as ro
lab = ro.IntVector(data_target[0:100].tolist())
lab_col = ro.StrVector(map(lambda p: p == 0 and 'blue' or 'red', lab))
lab_col.names = lab
ro.r.plot(x,y, xlab = "x", ylab="y", type="n")
ro.r.text(x,y,labels=lab, col = lab_col)#save_file = open(fileName, 'wb')  # this will overwrite current contents
#for rbm in sent_ae.stack:
#    cPickle.dump(rbm.W.get_value(borrow=True), save_file, -1)  # the -1 is for HIGHEST_PROTOCOL
#    cPickle.dump(rbm.vbias.get_value(borrow=True), save_file, -1)  # .. and it triggers much more efficient
#    cPickle.dump(rbm.hbias.get_value(borrow=True), save_file, -1)  # .. storage than numpy's default
#save_file.close()
