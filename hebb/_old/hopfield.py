
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt

import sys
sys.path.insert(0, "/home/alexeyche/prog/alexeyche-junk/cns/tf_dnn")

from util import xavier_init, xavier_vec_init, shl, shm, shs

np.random.seed(13)
tf.set_random_seed(6)

def initialize_layer(n_in, n_out):
    rng = np.random.RandomState()
    W_values = np.asarray(
        rng.uniform(
            low=-np.sqrt(6. / (n_in + n_out)),
            high=np.sqrt(6. / (n_in + n_out)),
            size=(n_in, n_out)
        ),
        dtype=np.float32
    )
    return W_values



def batch_inner(left, right):
	return tf.matmul(tf.expand_dims(left, 1), tf.expand_dims(right, 2))

def sparsity_term(V, p=0.1):
	return p * tf.log(tf.reduce_mean(V)+1e-07) + (1.0-p) * tf.log(1.0-tf.reduce_mean(V) + 1e-07)

def energy_function(W, sigma, V):
	# return - 0.5 * batch_inner(tf.matmul(V, W), V) - tf.matmul(V, sigma)
	# return 0.5 * tf.reduce_sum(batch_inner(V, V)) - 0.5 * batch_inner(tf.matmul(V, W), V) - tf.matmul(V, sigma)
	# return V * tf.reduce_sum(V) - 0.5 * batch_inner(tf.matmul(V, W), V) - tf.matmul(V, sigma)
	# return tf.reduce_sum(V) - 0.5 * batch_inner(tf.matmul(V, W), V) - tf.matmul(V, sigma)
	# return - sparsity_term(V) - 0.5 * batch_inner(tf.matmul(V, W), V) - tf.matmul(V, sigma) 
	return - alpha*sparsity_term(V)*V - 0.5 * batch_inner(tf.matmul(V, W), V) - tf.matmul(V, sigma) + tf.random_normal((batch_size, V.get_shape()[1].value))


def make_symmetrical(W):
	for i in xrange(W.shape[0]):
		for j in xrange(W.shape[1]):
			W[i, j] = W[j, i]
			if i == j:
				W[i, j] = 0.0

	return W


net_size = 100

alpha = net_size

input_size = 5

batch_size = 50

Wv = initialize_layer(net_size, net_size)
Wv = make_symmetrical(Wv)


x = tf.placeholder(tf.float32, [None, net_size], name="x")

W = tf.Variable(Wv, name="W", dtype=tf.float32)
sigma = tf.Variable(0.1*np.random.randn(net_size, 1), name="sigma", dtype=tf.float32)
# sigma = tf.Variable(np.zeros((net_size, 1)), name="sigma", dtype=tf.float32)

act = lambda x: tf.clip_by_value(x, 0.0, 1.0)
# act = lambda x0: (tf.sign(x0)+1.0)/2.0
# act = lambda x: tf.nn.sigmoid(x)



sess = tf.Session()
sess.run(tf.global_variables_initializer())


V = act(x)
E = energy_function(W, sigma, V)

dEdx = tf.gradients([tf.reduce_mean(E)], x)[0]

x_v = np.random.rand(batch_size, net_size).astype(np.float32)

input_v = np.random.random((10*batch_size, input_size))
input_v[input_v>0.1] = 0.0; input_v[input_v > 0.0] = 1.0
input_v = input_v[:batch_size, :]

def clamp_input(x):
	x[:, :input_size] = input_v
	return x

E_acc, x_acc, dE_acc = [], [], []
for e in xrange(2000):
	x_v = clamp_input(x_v)
	
	Ev, Vv, dEv = sess.run([E, V, dEdx], feed_dict={x: x_v})
	
	x_v = x_v - 0.05 * dEv
	
	print "Epoch {}, energy {}, mean dE {}, sparsity {}".format(e, np.mean(Ev), np.mean(dEv), np.mean(Vv))
	
	E_acc.append(Ev)
	x_acc.append(x_v.copy())
	dE_acc.append(dEv)

# E_acc = np.concatenate(E_acc)
x_acc = np.asarray(x_acc)
# dE_acc = np.concatenate(dE_acc)

# plt.quiver(x_acc[:,0], x_acc[:,1], -dE_acc[:,0], -dE_acc[:,1], width=0.001); plt.show()



# shl(dE_acc)