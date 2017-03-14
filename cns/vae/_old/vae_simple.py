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

from util import sm, sl, fun, kl_divergence_gauss, gmm_neg_log_likelihood

np.random.seed(10)
tf.set_random_seed(10)

epochs = 1000
lrate = 1e-03

batch_size = 300

weight_factor = 0.5

layers_num = 1

x_transformed = 100

phi_interm = 100
prior_interm = 100

z_dim = 10
z_interm = 100
out_interm = 100

input_dim = 60

input = tf.placeholder(tf.float32, shape=(batch_size, input_dim), name="Input")
input_z = tf.placeholder(tf.float32, shape=(batch_size, z_dim), name="InputZ")


def encode(x):
    x_t = fun(x, nout = x_transformed, act = tf.nn.relu, name = "x_transformed", weight_factor = weight_factor, layers_num = layers_num)

    phi = fun(x_t, nout = phi_interm, act = tf.nn.relu, name = "phi", weight_factor = weight_factor, layers_num = layers_num)
    z_mu = fun(phi, nout = z_dim, act = tf.identity, name = "z_mu", weight_factor = weight_factor)
    z_sigma = fun(phi, nout = z_dim, act = tf.nn.softplus, name = "z_sigma", weight_factor = weight_factor)

    epsilon = tf.random_normal((batch_size, z_dim), name='epsilon')
    z = z_mu + tf.exp(2.0 * z_sigma) * epsilon
    return z, z_mu, z_sigma

def decode(z):
    z_t = fun(z, nout = z_interm, act = tf.nn.relu, name = "z_transformed", weight_factor = weight_factor, layers_num = layers_num)

    output_t = fun(z_t, nout = out_interm, act = tf.nn.relu, name = "out_transform", weight_factor = weight_factor, layers_num = layers_num)
    post_mu = fun(output_t, nout = input_dim, act = tf.identity, name = "out_mu", weight_factor = weight_factor)
    post_sigma = fun(output_t, nout = input_dim, act = tf.nn.softplus, name = "out_sigma", weight_factor = weight_factor)
    post_alpha = fun(output_t, nout = input_dim, act = tf.nn.softmax, name = "out_alpha", weight_factor = weight_factor)
    return post_mu, post_sigma, post_alpha

with tf.variable_scope("vae") as scope:
    z, z_mu, z_sigma = encode(input)
    post_mu, post_sigma, post_alpha = decode(z)

rec_loss = tf.reduce_sum(tf.square(post_mu - input), 1)

z_prior_sigma = np.float32(1.0)
z_prior_mu = np.float32(0.0)

kl_loss = tf.reduce_sum(
    kl_divergence_gauss(z_mu, z_sigma, z_prior_mu, np.log(z_prior_sigma)), 
    1
)

recc_term = tf.reduce_sum(gmm_neg_log_likelihood(input, post_mu, post_sigma, post_alpha), 1)

loss = tf.reduce_mean(recc_term + kl_loss)

optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.AdadeltaOptimizer(lrate)
# optimizer = tf.train.RMSPropOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)

tvars = tf.trainable_variables()
grads = tf.gradients(loss, tvars)
# grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)
apply_grads = optimizer.apply_gradients(zip(grads, tvars))

sess = tf.Session()

synth_control_data_train = "/home/alexeyche/Music/ml/ucr_dataset/synthetic_control/synthetic_control_TRAIN"
# synth_control_data_train = "/Users/alexeyche/dnn/datasets/ucr/synthetic_control/synthetic_control_TRAIN"

data = np.loadtxt(synth_control_data_train)
classes = np.asarray(data[:,0], dtype=np.int)
input_v = data[:,1:]

sess.run(tf.global_variables_initializer())
stats = []
for e in xrange(epochs):
    sess_out = sess.run([
        post_mu,
        post_sigma,
        z_mu,
        z_sigma,
        kl_loss,
        rec_loss,
        loss,
        z,
        apply_grads
    ], {
        input: input_v
    })
    mu_v, sigma_v, z_mu_v, z_sigma_v, kl_v, rloss_v, loss_v, z_v, _ = sess_out
    stats.append((np.mean(kl_v), np.mean(z_mu_v), np.mean(z_sigma_v), np.mean(rloss_v), loss_v))
    print "Epoch {}, kl {} (mu {}), recc.loss {}, loss {}".format(e, np.mean(kl_v), np.mean(z_v), np.mean(rloss_v), loss_v)

stats = np.asarray(stats)


sl(stats[:,0], stats[:,1], np.exp(stats[:,2]), 0.1*stats[:,3], 0.1*stats[:,4], labels=["KL", "mu", "sigma", "recc.loss", "loss"])

with tf.variable_scope("vae", reuse=True) as scope:
    post_mu_v, post_sigma_v, post_alpha_v = sess.run(
        decode(input_z), {
        input_z: 2.0 * np.random.randn(batch_size, z_dim)
    })

