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
from avb_model import NormalDistribution, AvbModel, RunMode
from avb_model import HierarchicalDistribution

ds = tf.contrib.distributions

def sample_mog(batch_size, n_mixture=8, std=0.1, radius=1.0):
    thetas = np.linspace(0, 2 * np.pi, n_mixture)
    xs, ys = radius * np.sin(thetas), radius * np.cos(thetas)
    cat = ds.Categorical(tf.zeros(n_mixture))
    comps = [ds.MultivariateNormalDiag([xi, yi], [std, std]) for xi, yi in zip(xs.ravel(), ys.ravel())]
    data = ds.Mixture(cat, comps)
    return data.sample_n(batch_size)



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
config.weight_factor = 1.0



env = Env("avb")
env.clear_pics(env.run())

mode = RunMode.VAE

input = tf.placeholder(tf.float32, shape=(batch_size, config.input_dim), name="Input")

distribution = HierarchicalDistribution(
    NormalDistribution((batch_size, 4*config.z_dim), "normal0"),
    NormalDistribution((batch_size, config.z_dim), "normal1")
)


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

    adv_loss_v = 0.0
    if mode == RunMode.AVB:
        adv_loss_v, _ = sess.run([
            adv_loss,
            adv_step
        ], {
            input: test_data
        })
    
    print "Epoch {}, loss {}, neg ll {}, adv loss {}, kl value {}".format(e, net_loss_v, np.mean(neg_log_x_z_v), adv_loss_v, np.mean(kl_v))
    if e % 100 == 0:
        for idx, z_v in enumerate(z_samples_v):
            if z_v.shape[1] > 2:
                pca = dec.PCA(2)
                z_v = pca.fit(z_v).transform(z_v)
        
            plt.scatter(z_v[:,0], z_v[:,1])
            plt.savefig(env.run("z_scatter{}_{}.png".format(idx, e)))
            plt.clf()

        plt.scatter(post_mu_v[:,0], post_mu_v[:,1])
        plt.savefig(env.run("out_scatter{}.png".format(e)))
        plt.clf()





