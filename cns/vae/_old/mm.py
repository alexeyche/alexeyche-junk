#!/usr/bin/env python

import os
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob
from env import Env
import librosa as lr
import collections


from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell as rc
from tensorflow.python.ops import math_ops as mo
from tensorflow.python.ops import array_ops
from tensorflow.python.framework import dtypes

import sklearn.decomposition as dec
    
from util import sm, sl, smooth_matrix, smooth
from util import fun
from util import kl_divergence_gauss
from config import Config


def mm_inverse_cdf(epsilon, n=10):
    acc = tf.zeros(epsilon.shape)
    a_acc = tf.zeros(epsilon.shape)
    K = tf.zeros(epsilon.shape)
    for ki in xrange(n):
        with tf.variable_scope("a_scope"):
            a = tf.get_variable("a{}".format(ki), epsilon.get_shape(), dtype = tf.float32)
        
        K += a/n

    for ki in xrange(n):
        with tf.variable_scope("a_scope", reuse=True):
            a = tf.get_variable("a{}".format(ki), epsilon.get_shape(), dtype = tf.float32)
        
        a_prev = a_acc
        a_acc += a
        a_next = a_acc
        
        v0 = tf.where(e >= a_prev/K, e, tf.zeros(e.shape))
        
        
        v1 = tf.where(v0 <= a_next/K, v0, tf.zeros(e.shape))
        
        acc += v1 #* a # (ki/K + (K/a) * (e - start))
    return acc
    

lrate = 1e-01

sess = tf.Session()
e = tf.random_uniform((10, 10))
ans = mm_inverse_cdf(e)
step = tf.train.AdamOptimizer(lrate).minimize(tf.reduce_mean(ans))
tvars = tf.trainable_variables()
print [v.name for v in tvars]



sess.run(tf.global_variables_initializer())

ans_v, e_v = sess.run([ans, e])
print ans_v

for e in xrange(100):
    ans_v, _ = sess.run([ans, step])
    print np.mean(ans_v)
