
#!/usr/bin/env python

import os
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob
from env import Env

import collections


from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell as rc
from tensorflow.python.ops import math_ops as mo
from tensorflow.python.ops import array_ops
from tensorflow.python.framework import dtypes

from util import sm, sl, smooth_matrix, smooth, moving_average, norm, outer, generate_dct_dictionary

def xavier_init(fan_in, fan_out, constant=1):
    low = -constant*np.sqrt(6.0/(fan_in + fan_out))
    high = constant*np.sqrt(6.0/(fan_in + fan_out))
    return tf.random_uniform((fan_in, fan_out),
                             minval=low, maxval=high,
                             dtype=tf.float32)

np.random.seed(10)
tf.set_random_seed(10)

seq_size = 1000

batch_size = 1
filters_num = 90
L = 100
strides = 1
net_size = 100

h0_size = 125
h1_size = 125
latent_dim = 2
lam = 0

input = tf.placeholder(tf.float32, shape=(1, seq_size, batch_size, 1), name="Input")

env = Env("vae_run")


init = lambda shape, dtype: generate_dct_dictionary(L, h0_size).reshape(L, 1, 1, h0_size)

filter = tf.Variable(generate_dct_dictionary(L, h0_size).reshape(L, 1, 1, h0_size), name="F", dtype=tf.float32)

be0 = tf.Variable(np.zeros(h0_size), name="Be", dtype=tf.float32)
# We0 = tf.Variable(xavier_init(filters_num, h0_size), name="We0", dtype=tf.float32)

input_filtered = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='SAME')
input_filtered = tf.squeeze(input_filtered, 0)
input_filtered = tf.transpose(input_filtered, [1, 0, 2])
input_filtered = tf.squeeze(input_filtered, 0) # batch size = 1

# h0 = tf.nn.relu(tf.matmul(input_filtered, We0) + be0)
h0 = tf.nn.relu(input_filtered + be0)

be_mean = tf.Variable(np.zeros(latent_dim), name="BiasMean", dtype=tf.float32)
We_mean = tf.Variable(xavier_init(h0_size, latent_dim), name="WeMean", dtype=tf.float32)

be_var = tf.Variable(np.zeros(latent_dim), name="BiasVar", dtype=tf.float32)
We_var = tf.Variable(xavier_init(h0_size, latent_dim), name="WeVar", dtype=tf.float32)


mean_e = tf.matmul(h0, We_mean) + be_mean
var_e = tf.matmul(h0, We_var) + be_var


epsilon = tf.random_normal(tf.shape(var_e), name='epsilon')

std_encoder = tf.exp(0.5 * var_e)
z = mean_e + tf.mul(std_encoder, epsilon)

bd1 = tf.Variable(np.zeros(h1_size), name="Bd", dtype=tf.float32)
Wd1 = tf.Variable(xavier_init(latent_dim, h1_size), name="Wd1", dtype=tf.float32)

h1 = tf.nn.relu(tf.matmul(z, Wd1) + bd1)
h1 = tf.expand_dims(h1, 0)
h1 = tf.expand_dims(h1, 2)

filter_r = tf.Variable(generate_dct_dictionary(L, h1_size).reshape(L, 1, h1_size, 1), name="R", dtype=tf.float32)
input_hat = tf.nn.conv2d(h1, filter_r, strides=[1, strides, 1, 1], padding='SAME')

KLD = -0.5 * tf.reduce_sum(1 + var_e - tf.pow(mean_e, 2) - tf.exp(var_e), reduction_indices=1)
BCE = tf.reduce_sum(tf.nn.l2_loss(input_hat - input))

loss = tf.reduce_mean(BCE + KLD)

train_step = tf.train.AdamOptimizer(0.001).minimize(loss)

###############################################################



sess = tf.Session()

model_fname = env.run("model.ckpt")
saver = tf.train.Saver()
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())


tmp_dir = pj(os.environ["HOME"], "tmp", "tf_pics")
[ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]


input_v =  moving_average(np.random.randn(seq_size), 50).reshape(1, seq_size, batch_size, 1)

epochs = 1000
for e in xrange(epochs):
    h0_v, h1_v, input_filtered_v, input_hat_v, z_v, loss_v, KLD_v, _ = sess.run([
        h0, h1, input_filtered, input_hat, z, loss, KLD, train_step
    ], {
    	input: input_v
    })

    sl(input_v, input_hat_v, (input_v-input_hat_v) ** 2, file=pj(tmp_dir, "rec_{}.png".format(e)))
    sl(z_v, file=pj(tmp_dir, "latent_{}.png".format(e)))

    print "Epoch {}, loss {}, KLD {}".format(e, loss_v, np.mean(KLD_v))
