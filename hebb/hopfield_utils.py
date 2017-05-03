
import tensorflow as tf

def make_symmetrical(W):	
	for i in xrange(W.shape[0]):
		for j in xrange(W.shape[1]):
			W[i, j] = W[j, i]
			if i == j:
				W[i, j] = 0.0

	return W


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
