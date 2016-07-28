
import tensorflow as tf
from matplotlib import pyplot as plt

import numpy as np

w = 200
L = 100
batch_size = 1
input = tf.placeholder(tf.float32, shape=(batch_size, w, 1, 1), name="Input")

filter = tf.placeholder(tf.float32, shape=(L, 1, 1, 1), name="Filter")

op = tf.nn.conv2d(input, filter, strides=[1, 1, 1, 1], padding='VALID')

sess = tf.Session()    
sess.run(tf.initialize_all_variables())


input_v = np.zeros(w)
input_v[50] = 1.0

filter_v = np.exp(-np.linspace(0, 10, L))

op_v = sess.run(
	[op], {
		input: input_v.reshape(batch_size, w, 1, 1), 
		filter: filter_v.reshape(L, 1, 1, 1)
	}
)
