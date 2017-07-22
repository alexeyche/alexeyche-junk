
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

y = np.random.randn(5, 2)
x = np.random.randn(5, 10)

w = np.ones((10, 2))


dw = np.zeros(w.shape)

for i in xrange(dw.shape[0]):
	for j in xrange(dw.shape[1]):
		dw[i, j] += np.mean(y[:, j] * (x[:, i] - y[:, j] * w[i, j]))

vv = x - np.dot(y, w.T)
dw2 = np.dot(y.T, vv).T

dw2 = np.asarray([y[n] * v[:, n] for n in xrange(v.shape[1])]).T
	


# def calc_corr(net_v, variables):
# 	dws = []
# 	for li in xrange(1, net_v.shape[0]):
# 		li_prev = li - 1
# 		v = variables[li_prev]
# 		dw = np.zeros(v.shape)
		
		

# 		for bi in xrange(net_v.shape[1]):
# 			for i in xrange(dw.shape[0]):
# 				for j in xrange(dw.shape[1]):
# 					y = net_v[li, bi, j]
# 					x = net_v[li_prev, bi, i], 
# 					dw[i, j] += y * (x - y * v[i, j])

# 		dws.append(dw - np.mean(dw))
# 	return dws


# input_size = 10
# layer_size = 10
# layers_num = 10
# batch_size = 2000

# sizes = [(input_size, layer_size)] + [(layer_size, layer_size) for lid in xrange(layers_num-1)]

# x = tf.placeholder(tf.float32, (None, input_size), name="x")

# variables = [
#     tf.get_variable("W{}".format(lid), [size_pre, size_post], 
#         initializer=tf.constant_initializer(value=0.01)
#     )
#     for lid, (size_pre, size_post) in enumerate(sizes)
# ]

# biases = [
#     tf.get_variable("b{}".format(lid), [size_pre], 
#         initializer=tf.constant_initializer(value=-0.1)
#     )
#     for lid, (size_pre, _) in enumerate(sizes)
# ]


# net_signals = [x]
# for li, (v, b) in enumerate(zip(variables, biases)):
#     lin = tf.matmul(net_signals[-1], v) + b
#     v_norm = tf.sqrt(tf.reduce_sum(tf.square(v), 0))
	
#     lin = tf.Print(lin, [tf.reduce_mean(v_norm)], message="Layer {}, mean norm ".format(li))

#     lin += tf.random_normal((batch_size, v.get_shape()[1].value)) * (0.02/v_norm)

#     out = tf.nn.relu(lin)
#     net_signals.append(out)


# init = tf.global_variables_initializer()

# sess = tf.Session()
# sess.run(init)

# x_values, y_values = get_toy_data(input_size)

# for e in xrange(10):
# 	net_v = sess.run(net_signals, {x: x_values})

# 	net_v = np.asarray(net_v)
# 	dws = calc_corr(net_v, sess.run(variables))

# 	sess.run(tf.group(*[tf.assign_add(v, dv) for v, dv in zip(variables, dws)]))
	
# 	print "Epoch {}".format(e)	

# shm(np.mean(net_v, 1)[1:,])
