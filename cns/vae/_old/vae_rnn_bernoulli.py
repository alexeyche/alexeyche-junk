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
from random import shuffle

from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell as rc
from tensorflow.python.ops import math_ops as mo
from tensorflow.python.ops import array_ops
from tensorflow.python.framework import dtypes

from util import sm, sl, smooth_matrix, smooth
from util import moving_average, norm, fun, kl_divergence_gauss, gmm_neg_log_likelihood
from vae_model_bernoulli import VAECell
from config import Config


def generate_test_seq(seq_size=25, n_class=5):
    batch_size = 100

    # x_class_a = np.zeros((10, 10))
    # for i in xrange(10):
    #     x_class_a[i, i] = 1.0

    # x_class_b = np.zeros((10, 10))
    # for i in xrange(10):
    #     x_class_b[9-i, i] = 1.0
    

    classes, labs = [], []
    for i in xrange(n_class):
        x_class = np.zeros((seq_size, 10))
        x_class[np.where(0.1 > np.random.random((seq_size, 10)))] = 1.0
        classes += [x_class]*(batch_size/n_class)
        labs += [str(i)]*(batch_size/n_class)

    ids = range(0, len(classes))
    shuffle(ids)
    
    classes = [classes[id] for id in ids]
    labs = [labs[id] for id in ids]

    x_train = np.dstack(classes)
    x_train = np.transpose(x_train, (0,2,1))

    return x_train, labs


np.random.seed(10)
tf.set_random_seed(10)


x_train, x_labs = generate_test_seq()

epochs = 500
lrate = 1e-02
seq_size = x_train.shape[0]

batch_size = x_train.shape[1]

net_size = 20

config = Config()

config.z_interm = 20
config.phi_interm = 20
config.prior_interm = 20
config.out_interm = 20
config.n_out = x_train.shape[2]
config.z_dim = 2
config.x_transformed = 20

config.weight_factor = 0.2
config.layers_num = 1


env = Env("vae_run_toy")

generator = tf.placeholder(tf.bool, shape=(), name="generator")

cell = VAECell(tf.nn.rnn_cell.BasicRNNCell(net_size), generator, config)

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, config.n_out), name="Input")
state = tf.placeholder(tf.float32, [batch_size, net_size], name="state")

with tf.variable_scope("rnn") as scope:
    out, finstate = rnn.dynamic_rnn(cell, input, initial_state=state, time_major=True, scope = scope)

z_prior_mu, z_prior_sigma, z_mu, z_sigma, post_logit = out

# kl_loss = tf.reduce_sum(
#     - 0.5 + z_sigma - z_prior_sigma +
#     0.5 * (tf.exp(2.0 * z_prior_sigma) + tf.square(z_mu - z_prior_mu)) / tf.exp(2.0 * z_sigma),
#     [1, 2]
# )

# minimize Reverse-KL
kl_loss = tf.reduce_sum(
    kl_divergence_gauss(z_mu, z_sigma, z_prior_mu, z_prior_sigma),
    # kl_divergence_gauss(z_prior_mu, z_prior_sigma, z_mu, z_sigma),
    [1, 2]
)



recc_loss = tf.reduce_sum(tf.nn.sigmoid_cross_entropy_with_logits(post_logit, input), reduction_indices=[1, 2])

# error = 0.5 * tf.reduce_mean(tf.square(input_p - out_mu) / out_sigma**2 + 2 * tf.log(out_sigma) + tf.log(2 * np.pi), [1])

# error = tf.reduce_sum(tf.square(post_mu - input), 1)

loss = tf.reduce_mean(kl_loss + recc_loss)

optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.RMSPropOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)


tvars = tf.trainable_variables()
grads_raw = tf.gradients(loss, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

apply_grads = optimizer.apply_gradients(zip(grads, tvars))

sess = tf.Session()

model_fname = env.run("model.ckpt")
saver = tf.train.Saver()
if len(glob("{}*".format(model_fname))) > 0:
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 1000
else:
    sess.run(tf.global_variables_initializer())


tmp_dir = env.run()
tmp_grad_dir = env.run("grads")
if not os.path.exists(tmp_grad_dir):
    os.makedirs(tmp_grad_dir)

[ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]
[ os.remove(pj(tmp_grad_dir, f)) for f in os.listdir(tmp_grad_dir) if f[-4:] == ".png" ]

def sigmoid(x): return 1.0/(1.0 + np.exp(-x))

def generate(iterations=1):
    state_v = np.zeros((batch_size, net_size))

    output = []
    for i in xrange(iterations):
        out_gen_v, finstate_v = sess.run([
            out,
            finstate
        ], {
            input: np.zeros((seq_size, batch_size, config.n_out)),
            state: state_v,
            generator: True
        })
        state_v = finstate_v
        result_v = sigmoid(out_gen_v.post_logit)

        output.append(result_v)

    return np.concatenate(output)




for e in xrange(epochs):
    state_v = np.zeros((batch_size, net_size))

    output_data = []
    input_data = []
    perf = []

    sess_out = sess.run([
        finstate,
        out,
        kl_loss,
        recc_loss,
        loss,
        apply_grads,
        z_prior_mu, z_prior_sigma, z_mu, z_sigma
        # grads,
        # grads_raw,
        # out_sigma
    ], {
        input: x_train,
        state: state_v,
        generator: False
    })

    state_v, out_v, kl_v, error_v, loss_v, _ = sess_out[0:6]
    z_prior_mu_v, z_prior_sigma_v, z_mu_v, z_sigma_v = sess_out[6:10]
    result_v = out_v.post_logit
    
    output_data.append(result_v)
    input_data.append(x_train)
    perf.append((
        loss_v,
        np.mean(kl_v),
        np.mean(error_v),
        np.mean(np.square(z_prior_mu_v - z_mu_v)),
        np.mean(np.square(z_prior_sigma_v - z_sigma_v))
    ))

    sm(sigmoid(out_v.post_logit[:,0,:]), x_train[:,0,:], file=pj(tmp_dir, "rec_{}.png".format(e)))
    sl(*[out_v.mu[:,bi,:] for bi in xrange(batch_size)], file=pj(tmp_dir, "z_{}.png".format(e)))

    loss_v = np.mean(map(lambda x: x[0], perf))
    kl_v = np.mean(map(lambda x: x[1], perf))
    error_v = np.mean(map(lambda x: x[2], perf))
    mu_diff_v = np.mean(map(lambda x: x[3], perf))
    sigma_diff_v = np.mean(map(lambda x: x[4], perf))

    # for g, gr, tv in zip(grads_v, grads_raw_v, tvars):
    #     vname = tv.name.replace("/", "_").replace(":","_")

    #     f = pj(tmp_grad_dir, "{}_{}.png".format(vname, e))
    #     fr = pj(tmp_grad_dir, "{}_raw_{}.png".format(vname, e))

    #     work_dims = [s for s in g.shape if s > 1]
    #     if len(work_dims) == 1:
    #         sl(g, file=f)
    #         # sl(gr, file=fr)
    #     elif len(work_dims) == 2:
    #         sm(g, file=f)
    #         # sm(gr, file=fr)


    print "Epoch {}, loss {}, KLD {}, rec.error: {},\n\tmu diff: {}, sigma diff: {}".format(
        e, loss_v, kl_v, error_v, mu_diff_v, sigma_diff_v
    )

    if e % 10 == 0:
        d = generate()
        sm(d[:,0], file=pj(tmp_dir, "generated_{}.png".format(e)))
    
    

# saver.save(sess, model_fname)
