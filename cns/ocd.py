# -*- coding: utf-8 -*-

import numpy as np
import librosa as lr
import logging
from os.path import expanduser as pp
from matplotlib import pyplot as plt
from os.path import join as pj
import os

import tensorflow as tf
from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import array_ops

DEVICE = "gpu"

def gauss(x, mean, sd):
    return np.exp( - (x - mean)*(x - mean)/sd)

ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_list_files = []
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".npy"):
        data_list_files.append(pj(ds_dir, f))
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

source_id = 1


n_fft = 2048 # default
ms_frame = 10
sr = 22050
gauss_size = 1
gauss_sd = 1
hop = int(np.round(ms_frame * sr / 1000.0))


y, sr = lr.load(data_source[source_id])
y = y[11000:1000000]


L = 1000
N = 100
bs = 10
thr = 0.1

X = tf.placeholder(tf.float32, shape=(L, bs), name="Input")
D = vs.get_variable("D", (N, L))


# Ds = tf.sqrt(D ** 2 + 1e-8)
l2D = tf.sqrt(tf.reduce_sum(D ** 2, 1))
D = D / tf.expand_dims(l2D, 1)


# Xs = tf.sqrt(X ** 2 + 1e-8)
l2X = tf.sqrt(tf.reduce_sum(X ** 2, 0))
Xn = X / l2X

r = math_ops.matmul(D, Xn)
rm_v = math_ops.reduce_max(r, [0])

rm = tf.argmax(r, 0)
# rm = tf.reshape(rm, [bs, 1])
# print r_id_max.get_shape()
# print r.get_shape()

winner_filters = tf.gather(D, rm)
Xrest = Xn - rm_v * tf.transpose(winner_filters) 


# print r_max.get_shape()
# winner_ids = tf.where(r_max > thr)
# winners = tf.gather(r_max, winner_ids)

# idx = array_ops.constant(0, dtype=dtypes.int32, name="idx")


# def loop_body(i, r_id_max):


# tf.while_loop(
# 	cond = lambda idx, *_: idx < bs,
# 	body = loop_body,
# 	loop_vars = (idx, r_id_max)
# )



# train_step = tf.train.AdadeltaOptimizer(0.1).minimize(-r_max)

with tf.device("/{}:0".format(DEVICE)):
    sess = tf.Session()
    sess.run(tf.initialize_all_variables())
    # for e in xrange(100):
    id = 0
    # while id+L < y.shape[0]:
    d = np.zeros((L, bs))
    while id+L < y.shape[0]:
        d[:, id % bs] = y[id:id+L]
        id+=1
        if id % bs == 0:
        	break
    r_v, rm_v, Xn_v, Xrest_v = sess.run(
        [r, rm, Xn, Xrest], 
        {X: d}
    )
    # r_v, r_id_max_v, r_max_v, winner_ids_v, winners_v = sess.run(
    #     [r, r_id_max, r_max, winner_ids, winners], 
    #     {X: d}
    # )
		    # print "Epoch {}, cost {}".format(e, cost_v)

# plt.plot(d[:,0])
plt.plot(Xn_v[:,0])
plt.plot(Xrest_v[:,0])
plt.show()


# X = tf.placeholder(tf.float32, shape=(L, bs), name="Input")

# W = vs.get_variable("W", (N, L))

# f = math_ops.matmul(W, X)
# fs = tf.sqrt(f ** 2 + 1e-8)
# l2fs = tf.sqrt(tf.reduce_sum(fs ** 2, 1))
# nfs = fs / tf.expand_dims(l2fs, 1)
# l2fn = tf.sqrt(tf.reduce_sum(nfs ** 2, 0))
# f_hat = nfs / l2fn # tf.expand_dims(l2fn, 1)

# cost = tf.reduce_sum(tf.abs(f_hat))

# train_step = tf.train.AdadeltaOptimizer(0.1).minimize(cost)

# with tf.device("/{}:0".format(DEVICE)):
#     sess = tf.Session()
#     sess.run(tf.initialize_all_variables())
#     for e in xrange(100):
# 	    id = 0
# 	    while id+L < y.shape[0]:
# 		    d = np.zeros((L, bs))
# 		    while id+L < y.shape[0]:
# 		        d[:, id % bs] = y[id+L]
# 		        id+=1
# 		        if id % bs == 0:
# 		        	break

# 		    cost_v, _, f_hat_v = sess.run(
# 		        [cost, train_step, f_hat], 
# 		        {X: d}
# 		    )
# 		    print "Epoch {}, cost {}".format(e, cost_v)