
import tensorflow as tf
import numpy as np

def binary_setup(x_size, y_size, batch_size):
	y = np.random.random((10*batch_size, y_size))
	y[y>0.1] = 0.0; y[y > 0.0] = 1.0

	Wy = np.random.randn(y_size, x_size)
	y = y[np.where(np.sum(y, 1) != 0)]
	y = y[:batch_size, :]
	
	x = np.dot(y, Wy)
	return x, y


def linear_setup(x_size, y_size, batch_size):
	y = 0.1 * np.random.randn(batch_size, y_size)
	Wy = 0.1 * np.random.randn(y_size, x_size)
	x = np.dot(y, Wy)
	return x, y

def toy_setup(x_size, y_size, batch_size):
	y = np.zeros((batch_size, y_size))
	# for i in xrange(batch_size):
	# 	y[i,i % y_size] = 1.0
	
	y[batch_size/2,y_size/2] = 1.0

	x = np.zeros((batch_size, x_size))
	for i in xrange(batch_size):
		x[i,i % x_size] = 1.0
	return x, y

def symmetric_feedforward_weights(weights):
	sizes = [weights[0].get_shape()[0].value]
	sizes += [ w.get_shape()[0].value for w in weights[1:] ]
	sizes += [weights[-1].get_shape()[1].value]

	res = []
	for pre_id, pre_size in enumerate(sizes):
		stack = []
		for post_id, post_size in enumerate(sizes):
			if pre_id == post_id - 1 and pre_id < len(weights):
				stack.append(weights[pre_id])
			elif post_id == pre_id -1 and post_id < len(weights):
				stack.append(tf.transpose(weights[post_id]))
			else:
				pre_zeros = tf.zeros((pre_size, post_size))
				stack.append(pre_zeros)
		
		res.append(tf.concat_v2(stack, 1))


	res = tf.concat_v2(res, 0)

	# for i in xrange(res_v.shape[0]):
	# 	for j in xrange(res_v.shape[1]):
	# 		assert res_v[i, j] == res_v[j, i]

	return res

def batch_outer(left, right):
	return tf.matmul(tf.expand_dims(left, 1), tf.expand_dims(right, 2))
