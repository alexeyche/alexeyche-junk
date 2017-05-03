
from tensorflow.examples.tutorials.mnist import input_data

import tensorflow as tf
import os
import sys
import numpy as np

sys.path.insert(0, "/home/alexeyche/prog/alexeyche-junk/cns/tf_dnn")

from util import xavier_init, xavier_vec_init, shl, shm
from function import function 

weight_factor = 0.1
init = lambda shape, dtype, partition_info: xavier_init(shape[0], shape[1], const = weight_factor)
bias_init = lambda shape, dtype, partition_info: np.zeros((shape[0],))
small_init = lambda shape, dtype, partition_info: xavier_init(shape[0], shape[1], const = 0.001)
small_vec_init = lambda shape, dtype, partition_info: xavier_vec_init(shape[0], const = 0.001)
vec_init = lambda shape, dtype, partition_info: xavier_vec_init(shape[0], const = weight_factor)
zero_init = lambda shape, dtype, partition_info: np.zeros((shape[0], shape[1]))

mnist = input_data.read_data_sets(os.path.join(os.environ["HOME"], "mnist"), one_hot=True)

input_size = 784
h0_size = 500
h1_size = 500
output_size = 10
batch_size = 100
episodes_num = 5

# Create the model
x = tf.placeholder(tf.float32, [None, input_size])
y_ = tf.placeholder(tf.float32, [None, 10])

W0 = tf.get_variable("W0", [input_size, h0_size], initializer = init, dtype=tf.float32)
# W1 = tf.get_variable("W1", [h0_size, output_size], initializer = init, dtype=tf.float32)

W1 = tf.get_variable("W1", [h0_size, h1_size], initializer = init, dtype=tf.float32)
W2 = tf.get_variable("W2", [h1_size, output_size], initializer = init, dtype=tf.float32)

# Hc0 = tf.get_variable("Hc0", [input_size, h0_size], initializer = init, dtype=tf.float32)
# Hx0 = tf.get_variable("Hx0", [input_size], initializer = vec_init, dtype=tf.float32)
# Hy0 = tf.get_variable("Hy0", [h0_size], initializer = vec_init, dtype=tf.float32)
# Hx20 = tf.get_variable("Hx20", [input_size], initializer = vec_init, dtype=tf.float32)
# Hy20 = tf.get_variable("Hy20", [h0_size], initializer = vec_init, dtype=tf.float32)


# Hc1 = tf.get_variable("Hc1", [h0_size, h1_size], initializer = init, dtype=tf.float32)
# Hx1 = tf.get_variable("Hx1", [h0_size], initializer = vec_init, dtype=tf.float32)
# Hy1 = tf.get_variable("Hy1", [output_size], initializer = vec_init, dtype=tf.float32)
# Hx21 = tf.get_variable("Hx21", [h0_size], initializer = vec_init, dtype=tf.float32)
# Hy21 = tf.get_variable("Hy21", [output_size], initializer = vec_init, dtype=tf.float32)


# Hc1 = tf.get_variable("Hc1", [h0_size, h1_size], initializer = small_init, dtype=tf.float32)
# Hc2 = tf.get_variable("Hc2", [h1_size, output_size], initializer = small_init, dtype=tf.float32)

cross_entropy_list, accuracy_list = [], []

for episode_num in xrange(episodes_num):
	h0 = tf.nn.relu(tf.matmul(x, W0))

	######
	hebb0 = 0.1 * tf.matmul(
		tf.transpose(x), 
		function(
			h0, 
			name="h0", 
			size=h0_size, 
			scope="h0", 
			reuse=(True if episode_num > 0 else False), 
			layers_num=1, 
			act=tf.nn.relu,
			weight_factor=0.0001
		)
	)
	
	W0 += hebb0

	######

	h1 = tf.nn.relu(tf.matmul(h0, W1))

	hebb1 = 0.1 * tf.matmul(
		tf.transpose(h0), 
		function(
			h1, 
			name="h1", 
			size=h1_size, 
			scope="h1", 
			reuse=(True if episode_num > 0 else False), 
			layers_num=1, 
			act=tf.nn.relu,
			weight_factor=0.0001
		)
	)
	
	# hebb1 = (
	# 	Hc1 * tf.matmul(tf.transpose(h0), h1) - tf.matmul(tf.expand_dims(tf.reduce_mean(h0, 0), 1), tf.expand_dims(tf.reduce_mean(h1, 0), 0))
	# 	# tf.expand_dims(Hx1 * tf.reduce_mean(h0, 0), 1) + 
	# 	# tf.expand_dims(Hy1 * tf.reduce_mean(y, 0), 0) + 
	# 	# tf.expand_dims(Hx21 * tf.reduce_mean(h0, 0) * tf.reduce_mean(h0, 0), 1) + 
	# 	# tf.expand_dims(Hy21 * tf.reduce_mean(y, 0) * tf.reduce_mean(y, 0), 0)
	# )
	
	W1 += hebb1

	y = tf.matmul(h1, W2)

	ep_cross_entropy = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(labels=y_, logits=y))

	ep_correct_prediction = tf.equal(tf.argmax(y, 1), tf.argmax(y_, 1))
	ep_accuracy = tf.reduce_mean(tf.cast(ep_correct_prediction, tf.float32))

	cross_entropy_list.append(ep_cross_entropy)
	accuracy_list.append(ep_accuracy)

cross_entropy = tf.reduce_mean(tf.stack(cross_entropy_list))
accuracy = tf.reduce_mean(tf.stack(accuracy_list))

opt = tf.train.AdamOptimizer(0.0001)
# opt = tf.train.RMSPropOptimizer(0.001)
# opt = tf.train.GradientDescentOptimizer(0.01)

# tvars = tf.trainable_variables()
tvars = [v for v in tf.trainable_variables() if not (str(v.name).startswith("W0") or str(v.name).startswith("W1"))] 

grads = tf.gradients(cross_entropy, tvars)
train_step = opt.apply_gradients(zip(grads, tvars))


sess = tf.Session()
sess.run(tf.global_variables_initializer())

curves = []
for e in range(1000):
	batch_xs, batch_ys = mnist.train.next_batch(batch_size)
	
	ce, train_acc, _ = sess.run(
		[
			cross_entropy, 
			accuracy, 
			train_step
		], 
		feed_dict={x: batch_xs, y_: batch_ys}
	)
	
	test_acc = sess.run(accuracy, feed_dict={x: mnist.test.images, y_: mnist.test.labels})

	print "Epoch {}, ce: {}, train acc {}, test acc {}".format(e, ce, train_acc, test_acc)
	
	curves.append((ce, train_acc, test_acc))

# shl([c[0] for c in curves], [c[1] for c in curves])


# hebb0_v = sess.run([hebb0], feed_dict={x: batch_xs, y_: batch_ys})
