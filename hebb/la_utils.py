
import tensorflow as tf

def batch_inner(left, right):
	return tf.matmul(tf.expand_dims(left, 1), tf.expand_dims(right, 2))
