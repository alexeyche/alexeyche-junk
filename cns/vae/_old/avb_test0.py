#!/usr/bin/env python

from log import setup_log

import os
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob
from env import Env
import librosa as lr
import collections
import logging

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
from avb_model import NormalDistribution, AvbModel, RunMode
from avb_model import HierarchicalDistribution
from avb_model import BernoulliDistribution
from avb_model import plot_poiss, sample_mog

ds = tf.contrib.distributions


np.random.seed(10)
tf.set_random_seed(10)

epochs = 2500

batch_size = 1000

config = Config()

config.lrate = 1e-02
config.z_dim = 2
config.input_dim = 2
config.z_interm = 25
config.x_interm = 25
config.a_interm = 25
config.output_dim = config.input_dim
config.layers_num = 2
config.weight_factor = 0.25



env = Env("avb")
env.clear_pics(env.run())

mode = RunMode.VAE

input = tf.placeholder(tf.float32, shape=(batch_size, config.input_dim), name="x")

# distribution = HierarchicalDistribution(
#     # BernoulliDistribution((batch_size, 10*config.z_dim), "bernoulli0"),
#     NormalDistribution((batch_size, config.z_dim), "normal0"),
#     NormalDistribution((batch_size, config.z_dim), "normal1")
# )

distribution = NormalDistribution((batch_size, config.z_dim), "normal0")
    


model = AvbModel(config, distribution)

z, z_prior, neg_log_x_z, kl, adv_prior, post_mu, net_loss, adv_loss, net_step, adv_step = AvbModel.build(model, input, mode)



sess = tf.Session()
sess.run(tf.global_variables_initializer())

test_data = sess.run(sample_mog(batch_size))

for e in xrange(epochs):    
    z_samples_v, z_prior_samples_v, neg_log_x_z_v, kl_v, adv_prior_v, post_mu_v,  net_loss_v, _ = sess.run([
        z, 
        z_prior, 
        neg_log_x_z,
        kl,
        adv_prior,
        post_mu,
        net_loss, 
        net_step,
    ], {
        input: test_data
    })

    adv_loss_v, _ = sess.run([
        adv_loss,
        adv_step
    ], {
        input: test_data
    })
        
    if e % 100 == 0:
        logging.info("Epoch {}, loss {}, neg ll {}, adv loss {}, kl value {}".format(e, net_loss_v, np.mean(neg_log_x_z_v), adv_loss_v, np.mean(kl_v)))
        for idx, z_v in enumerate(z_samples_v):
            if z_v.shape[1] > 2:
                pca = dec.PCA(2)
                z_v = pca.fit(z_v).transform(z_v)
        
            plt.scatter(z_v[:,0], z_v[:,1])
            plt.savefig(env.run("z_scatter{}_{}.png".format(idx, e)))
            plt.clf()

        # # plot_poiss(z_samples_v[0])
        # plt.scatter(z_samples_v[0][:,0], z_samples_v[0][:,1])
        # plt.savefig(env.run("z_scatter{}_{}.png".format(0, e)))
        # plt.clf()

        # plt.scatter(z_samples_v[1][:,0], z_samples_v[1][:,1])
        # plt.savefig(env.run("z_scatter{}_{}.png".format(1, e)))
        # plt.clf()



        plt.scatter(post_mu_v[:,0], post_mu_v[:,1])
        plt.savefig(env.run("out_scatter{}.png".format(e)))
        plt.clf()





