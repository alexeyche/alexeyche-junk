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

from util import sm, sl, smooth_matrix, smooth
from util import fun
from util import kl_divergence_gauss
from config import Config


class RunMode(object):
    VAE = "vae_mode"
    AVB = "avb_mode"

np.random.seed(10)
tf.set_random_seed(10)

epochs = 10000
lrate = 1e-03

batch_size = 99

config = Config()

config.z_dim = 2
config.input_dim = 8
config.z_interm = 256
config.x_interm = 256
config.a_interm = 256
config.output_dim = config.input_dim

config.weight_factor = 1.0

env = Env("avb")
env.clear_pics(env.run())

mode = RunMode.AVB

input = tf.placeholder(tf.float32, shape=(batch_size, config.input_dim), name="Input")

def distribution(shape):
    epsilon = tf.random_normal(shape)
    # epsilon = tf.truncated_normal(shape)
    # epsilon = tf.random_uniform(shape)

    # epsilon = tf.random_gamma(shape, 0.1, 1.0/0.1)

    # epsilon = tf.nn.softplus(epsilon)
    return epsilon

def encoder(x):
    x_t = fun(x, nout = config.x_interm, act = tf.nn.relu, name = "encode_x_transformed", layers_num=2, config=config)
    z_mu = fun(x_t, nout = config.z_dim, act = tf.identity, name = "encode_z_mu")
    z_sigma = fun(x_t, nout = config.z_dim, act = tf.nn.softplus, name = "encode_z_sigma")
    epsilon = distribution((batch_size, config.z_dim))
    z = z_mu + tf.sqrt(tf.exp(z_sigma)) * epsilon
    return z, z_mu, z_sigma

def decoder(z):
    z_t = fun(z, nout = config.z_interm, act = tf.nn.relu, name = "decode_z_transformed", layers_num=2, config=config)
    post_mu = fun(z_t, nout = config.output_dim, act = tf.identity, name = "decode_out_mu")
    return post_mu

def discriminator(x, z, reuse=False):
    a_t = fun(x, z, nout = config.a_interm, act = tf.nn.relu, name = "discriminator_transformed", layers_num=2, config=config, reuse=reuse)
    a_out = fun(a_t, nout = config.output_dim, act = tf.sigmoid, name = "discriminator_out", reuse=reuse)
    return a_out


z_prior = distribution((batch_size, config.z_dim))

z, z_mu, z_sigma = encoder(input)

post_mu = decoder(z)

adv = discriminator(input, z)
adv_prior = discriminator(input, z_prior, reuse=True)

# neg_log_x_z = tf.nn.sigmoid_cross_entropy_with_logits(post_mu, input)
neg_log_x_z = tf.nn.l2_loss(post_mu - input)

if mode == RunMode.VAE:
    kl = kl_divergence_gauss(z_mu, z_sigma, np.float32(0.0), np.float32(0.0))
elif mode == RunMode.AVB:
    kl = adv    
else:
    raise Exception("Unknown mode")


net_loss = tf.reduce_mean(kl + neg_log_x_z)
adv_loss = tf.reduce_mean(-tf.log(adv+1e-10) - tf.log(1.0 - adv_prior + 1e-10))

vars = tf.trainable_variables()
net_params = [v for v in vars if not v.name.startswith('discriminator')]
adv_params = [v for v in vars if v.name.startswith('discriminator')]

net_step = tf.train.AdamOptimizer(lrate).minimize(net_loss, var_list=net_params)
adv_step = tf.train.AdamOptimizer(lrate).minimize(adv_loss, var_list=adv_params)

sess = tf.Session()
sess.run(tf.global_variables_initializer())

test_data = []
test_data_classes = []
for f1, f2, f3, f4, f5, f6, f7, f8, cl in np.recfromcsv(env.dataset("IRIS.csv"), delimiter=","):
    test_data.append(np.asarray([[f1, f2, f3, f4, f5, f6, f7, f8]]))
    test_data_classes.append(cl)

test_data = np.concatenate(test_data)
uniq_classes = list(set(test_data_classes))
test_data_classes = [ uniq_classes.index(cl) for cl in test_data_classes ]


# test_data = np.asarray([
#     [1.0, 1.0, 1.0, 0.0],
#     [1.0, 1.0, 0.0, 1.0],
#     [1.0, 0.0, 1.0, 1.0],
#     [0.0, 1.0, 1.0, 1.0]
# ])
# test_data_classes = [0, 1, 2, 3]


for e in xrange(epochs):
    z_v, z_prior_v, adv_v, adv_prior_v, post_mu_v,  net_loss_v, _ = sess.run([
        z, 
        z_prior, 
        adv,
        adv_prior,
        post_mu,
        net_loss, 
        net_step
    ], {
        input: test_data
    })

    adv_loss_v = 0.0
    if mode == RunMode.AVB:
        adv_loss_v, _ = sess.run([
            adv_loss,
            adv_step
        ], {
            input: test_data
        })
    

    print "Epoch {}, net loss {}, adv loss {}, value {}".format(e, net_loss_v, adv_loss_v, np.mean(adv_v))
    if e % 100 == 0:

        z_acc, cols = [], []
        for i in xrange(100):
            z_v = sess.run([z], {input: test_data})[0]
            z_acc.append(z_v)
            cols.append(test_data_classes)

        z_v = np.concatenate(z_acc)
        plt.scatter(z_v[:,0], z_v[:,1], c=cols)
        plt.savefig(env.run("scatter{}.png".format(e)))
        plt.clf()






