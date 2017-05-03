
import tensorflow as tf
import numpy as np
from hopfield_util import symmetric_feedforward_weights, batch_outer, toy_setup, binary_setup, linear_setup

import sys
sys.path.insert(0, "/home/alexeyche/prog/alexeyche-junk/cns/tf_dnn")

from util import xavier_init, xavier_vec_init, shl, shm

np.random.seed(4)
tf.set_random_seed(4)

weight_factor = 0.1
init = lambda shape, dtype, partition_info: xavier_init(shape[0], shape[1], const = weight_factor)
bias_init = lambda shape, dtype, partition_info: np.zeros((shape[0],))


net_size = 30
input_size = 10
output_size = 10
hidden_size = 10

x = tf.placeholder(tf.float32, [None, input_size], name="x")
u = tf.placeholder(tf.float32, [None, net_size], name="u")
y = tf.placeholder(tf.float32, [None, output_size], name="y")

clamp_input = lambda ul: tf.concat_v2([x, tf.slice(ul, [0, input_size], [-1, net_size - input_size])], 1)

cost = lambda xl: 0.5*tf.reduce_sum(tf.square(y - tf.slice(xl, [0, net_size - output_size], [-1, output_size])))
act = lambda x: tf.clip_by_value(x, 0.0, 1.0)
# act = lambda x: x

W0 = tf.get_variable("W0", [input_size, hidden_size], initializer = init, dtype=tf.float32)
b0 = tf.get_variable("b0", [hidden_size], initializer = bias_init, dtype=tf.float32)

W1 = tf.get_variable("W1", [hidden_size, output_size], initializer = init, dtype=tf.float32)
b1 = tf.get_variable("b1", [output_size], initializer = bias_init, dtype=tf.float32)

biases = tf.expand_dims(tf.concat_v2([tf.zeros(input_size), b0, b1], 0), 1)

W = symmetric_feedforward_weights([W0, W1])


u_clamp = clamp_input(u)
p = act(u_clamp)

E = (
	tf.reduce_sum(batch_outer(p, p)) 
 	- 0.5 * tf.reduce_sum(batch_outer(tf.matmul(p, W), p))
 	- tf.reduce_sum(tf.matmul(p, biases))
)



F = E #+ cost(p)
# F = E

dEdu = tf.gradients([F], u_clamp)[0]

u_v = np.zeros((1, net_size,))
x_v = np.zeros((1, input_size,))
# u_v = np.random.randn(1, net_size,)
y_v = np.zeros((1, output_size,))

y_v[0, -2] = 1.0
x_v[0, 0] = 1.0
x_v[0, 3] = 1.0
# x_v[0, 6] = 1.0
# x_v[0, 9] = 1.0


sess = tf.Session()
sess.run(tf.global_variables_initializer())

u_acc = []

for e in xrange(50):
	Fv, dEv, cost_v = sess.run([F, dEdu, cost(p)], feed_dict={u: u_v, y: y_v, x: x_v})
	u_v = u_v - 0.5 * dEv
	print "step {}, F {:0.3f}, E[dEds] {:0.3f}, cost {:0.3f}".format(e, Fv, np.mean(dEv), cost_v)

	u_acc.append(u_v)

u_acc = np.concatenate(u_acc)	