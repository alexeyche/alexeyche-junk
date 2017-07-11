
import time

from util import shl, shm, shs
from sklearn.datasets import make_classification

import numpy as np

import tensorflow as tf

def get_toy_data(n_features, size=2000, seed=2):
    x_values, y_values = make_classification(        
        n_samples=size,
        n_features=n_features, 
        random_state=seed
    )
    return x_values, y_values


dog_init = lambda shape, dtype, partition_info: build_dog(shape[0], shape[1], amp_neg=0.0)

input_size = 10
layer_size = 10
layers_num = 10
batch_size = 2000

sizes = [(input_size, layer_size)] + [(layer_size, layer_size) for lid in xrange(layers_num-1)]

x = tf.placeholder(tf.float32, (None, input_size), name="x")

variables = [
    tf.get_variable("W{}".format(lid), [size_pre, size_post], 
        initializer=tf.constant_initializer(value=0.01)
    )
    for lid, (size_pre, size_post) in enumerate(sizes)
]

biases = [
    tf.get_variable("b{}".format(lid), [size_pre], 
        initializer=tf.constant_initializer(value=-0.1)
    )
    for lid, (size_pre, _) in enumerate(sizes)
]


net_signals = [x]
for v, b in zip(variables, biases):
    lin = tf.matmul(net_signals[-1], v) + b
    
    lin += tf.random_normal((batch_size, v.get_shape()[1].value), mean=0.0, stddev=0.5)

    out = tf.nn.relu(lin)
    net_signals.append(out)


init = tf.global_variables_initializer()

sess = tf.Session()
sess.run(init)

x_values, y_values = get_toy_data(input_size)

net_v = sess.run(net_signals, {x: x_values})

net_v = np.asarray(net_v)

shm(np.mean(net_v, 1)[1:,])