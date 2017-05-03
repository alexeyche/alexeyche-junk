
import tensorflow as tf
import numpy as np

from hopfield_utils import make_symmetrical
from nn_utils import initialize_layer
from energy import SparseEnergy, WtaEnergy, ExpDecayEnergy

import sys
sys.path.insert(0, "/home/alexeyche/prog/alexeyche-junk/cns/tf_dnn")

from util import xavier_init, xavier_vec_init, shl, shm

np.random.seed(13)
tf.set_random_seed(6)


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

# energy = SparseEnergy(W, sigma, p=0.1)
energy = WtaEnergy(W, sigma, input_size)

act = lambda x: tf.clip_by_value(x, 0.0, 1.0)
# act = lambda x0: (tf.sign(x0)+1.0)/2.0
# act = lambda x: tf.nn.sigmoid(x)



sess = tf.Session()
sess.run(tf.global_variables_initializer())


V = act(x)
E = energy(V)

dEdx = tf.gradients([tf.reduce_mean(E)], x)[0]

x_v = np.random.rand(batch_size, net_size).astype(np.float32)
# x_v = np.random.rand(batch_size, net_size).astype(np.float32)

input_v = np.random.random((10*batch_size, input_size))
input_v[input_v>0.1] = 0.0; input_v[input_v > 0.0] = 1.0
input_v = input_v[:batch_size, :]

def clamp_input(x):
	x[:, :input_size] = input_v
	return x

E_acc, x_acc, dE_acc = [], [], []
for e in xrange(4):
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