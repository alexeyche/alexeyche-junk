
import tensorflow as tf
from matplotlib import pyplot as plt

import numpy as np

w = 200
L = 100
batch_size = 1
input = tf.placeholder(tf.float32, shape=(batch_size, w, 1, 1), name="Input")

delta = tf.placeholder(tf.float32, shape=(1, L+1, 1, 1), name="Delta")

filter = tf.placeholder(tf.float32, shape=(L, 1, 1, 1), name="Filter")

op = tf.nn.conv2d(input, filter, strides=[1, 1, 1, 1], padding='VALID')

recov_filter = tf.nn.conv2d_transpose(delta, filter, output_shape = (1, 2*L, 1, 1), strides=[1, 1, 1, 1], padding='VALID')

op_recov = tf.nn.conv2d_transpose(op, filter, output_shape = (batch_size, w, 1, 1), strides=[1, 1, 1, 1], padding='VALID')

sess = tf.Session()
sess.run(tf.initialize_all_variables())


input_v = np.zeros(w)
input_v[50] = 1.0


delta_v = np.zeros(L+1)
delta_v[-1] = 1.0


filter_v = np.exp(-np.linspace(0, 20, L))

op_v, op_recov_v, recov_filter_v = sess.run(
    [op, op_recov, recov_filter], {
        input: input_v.reshape(batch_size, w, 1, 1),
        filter: filter_v.reshape(L, 1, 1, 1),
        delta: delta_v.reshape(1, L+1, 1, 1)
    }
)
