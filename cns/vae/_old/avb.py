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
from avb_model import NormalDistribution, BernoulliDistribution, HierarchicalDistribution, AvbModel, RunMode
from avb_model import plot_poiss, sample_mog
import logging

np.random.seed(10)
tf.set_random_seed(10)

epochs = 400

batch_size = 99

config = Config()

config.lrate = 1e-02
config.z_dim = 2
config.input_dim = 8
config.z_interm = 25
config.x_interm = 25
config.a_interm = 25
config.output_dim = config.input_dim
config.layers_num = 2
config.weight_factor = 0.1



env = Env("avb")
env.clear_pics(env.run())

mode = RunMode.VAE

input = tf.placeholder(tf.float32, shape=(batch_size, config.input_dim), name="x")

# distribution = HierarchicalDistribution(
#     NormalDistribution((batch_size, 5*config.z_dim), "normal0"),
#     NormalDistribution((batch_size, config.z_dim), "normal1")
# )

# distribution = HierarchicalDistribution(
#     BernoulliDistribution((batch_size, 10*config.z_dim), "b0"),
#     NormalDistribution((batch_size, config.z_dim), "normal0")
# )

distribution = NormalDistribution((batch_size, config.z_dim))

model = AvbModel(config, distribution)

z, z_prior, neg_log_x_z, kl, adv_prior, post_mu, net_loss, adv_loss, net_step, adv_step = AvbModel.build(model, input, mode, full_sample=False)


sess = tf.Session()
sess.run(tf.global_variables_initializer())

test_data = []
test_data_classes = []
for f1, f2, f3, f4, f5, f6, f7, f8, cl in np.recfromcsv(env.dataset("IRIS.csv"), delimiter=","):
    test_data.append(np.asarray([[f1, f2, f3, f4, f5, f6, f7, f8]]))
    test_data_classes.append(cl)

test_data = np.concatenate(test_data)
uniq_classes = list(set(test_data_classes))
test_data_classes = np.asarray([ uniq_classes.index(cl) for cl in test_data_classes ])


writer = tf.summary.FileWriter("{}/tf_old".format(os.environ["HOME"]), graph=tf.get_default_graph())


# test_data = np.asarray([
#     [1.0, 1.0, 1.0, 0.0],
#     [1.0, 1.0, 0.0, 1.0],
#     [1.0, 0.0, 1.0, 1.0],
#     [0.0, 1.0, 1.0, 1.0]
# ])
# test_data_classes = [0, 1, 2, 3]

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
    
    if e % 10 == 0:
        logging.info("Epoch {}, loss {}, neg ll {}, adv loss {}, value {}".format(e, net_loss_v, np.mean(neg_log_x_z_v), adv_loss_v, np.mean(kl_v)))

        # plot_poiss(z_samples_v[0], c=test_data_classes)
        # plt.savefig(env.run("z_scatter_p_{}.png".format(e)))
        # plt.clf()
        # z_samples_v.pop(0)

        for idx, z_v in enumerate(z_samples_v):
            if z_v.shape[1] > 2:
                pca = dec.PCA(2)
                z_v = pca.fit(z_v).transform(z_v)
        
            plt.scatter(z_v[:,0], z_v[:,1], c=test_data_classes)
            plt.savefig(env.run("z_scatter{}_{}.png".format(idx, e)))
            plt.clf()

      



