
import tensorflow as tf
import numpy as np
from matplotlib import pyplot as plt

from util import shm, shl

def generate_toy_problem(p_size, batch_size, rank=5, card=0.20):
	basis = np.random.random((rank, p_size))

	X0 = np.zeros((batch_size, p_size))

	for i in xrange(batch_size):
		ind = int(np.floor(np.random.random()*rank ))
		X0[i, :] = basis[ind, :]

	X1 = np.sign(np.random.random((batch_size, p_size))- 0.5)
	X1 *= np.random.random((batch_size, p_size)) < card

	return X0, X1


def batch_outer(left, right):
    return np.asarray([np.outer(left[i], right[i]) for i in xrange(left.shape[0])])


p_size = 10
r_size = 9

lambda1 = 1.0
lambda2 = 1.0

batch_size = 1

zeros_init = lambda shape, dtype, partition_info: np.zeros(shape)

rng = np.random.RandomState()

bengio_init = lambda shape, dtype, partition_info: np.asarray(
    rng.uniform(
        low=-np.sqrt(6. / (shape[0] + shape[1])),
        high=np.sqrt(6. / (shape[0] + shape[1])),
        size=shape
    ),
    dtype=np.float32
)


z = tf.placeholder(dtype=tf.float32, shape=[None, p_size], name="z")

r = tf.get_variable("r", [batch_size, r_size], dtype = tf.float32, initializer = bengio_init)
L = tf.get_variable("L", [p_size, r_size], dtype = tf.float32, initializer = bengio_init)
e = tf.get_variable("e", [batch_size, p_size], dtype = tf.float32, initializer = zeros_init)




cost = (
	tf.nn.l2_loss(z - tf.transpose(tf.matmul(L, tf.transpose(r))) - e) + 
	lambda1 * tf.nn.l2_loss(r) + 
	lambda2 * tf.reduce_sum(tf.abs(e)) + 
	lambda1 * tf.sqrt(tf.nn.l2_loss(L))
)


optimizer = tf.train.AdamOptimizer(0.001)
# optimizer = tf.train.GradientDescentOptimizer(0.01)
# optimizer = tf.train.MomentumOptimizer(0.0001, 0.9, use_nesterov=True)
# optimizer = tf.train.FtrlOptimizer(0.00001) # not bad
# optimizer = tf.train.RMSPropOptimizer(0.000001)
# optimizer = tf.train.AdagradOptimizer(0.0001)
# optimizer = tf.train.AdadeltaOptimizer(0.0001)
# optimizer = tf.train.ProximalAdagradOptimizer(0.0001)


tvars = [r, e, L]

grads = tf.gradients(cost, tvars)
apply_grads = optimizer.apply_gradients(zip(grads, tvars))


X0, noise_val = generate_toy_problem(p_size, batch_size)

z_v = X0 + noise_val

At = np.zeros((r_size, r_size))
Bt = np.zeros((p_size, r_size))



sess = tf.Session()

sess.run(tf.global_variables_initializer())

for e_id in xrange(3000):
	cost_v, r_v, e_v, L_v, grads_v, _ = sess.run([cost, r, e, L, grads, apply_grads], {z: z_v})
	print "Epoch {}, cost {}".format(e_id, cost_v)

	# for s_id in xrange(100):
	# 	cost_v, r_v, e_v, L_v, grads_v, _ = sess.run([cost, r, e, L, grads, apply_grads], {z: z_v})
	# 	print "Session {}, cost {}".format(s_id, cost_v)

	# At += np.mean(batch_outer(r_v, r_v), 0)
	# Bt += np.mean(batch_outer(z_v-e_v, r_v), 0)

	# Atilda = At + lambda1 * np.eye(*At.shape)
	# for ri in xrange(r_size):
	# 	L_v[:, ri] += (Bt[:, ri] - np.dot(L_v, Atilda[:, ri])) /Atilda[ri, ri] 

	# sess.run(tf.assign(L, L_v))
	
	# print "Epoch {}, cost {}".format(e_id, cost_v)