
import tensorflow as tf
import os
import sys
import numpy as np
from functools import partial
from os.path import join as pj

sys.path.insert(0, "/home/alexeyche/prog/alexeyche-junk/cns/tf_dnn")

from util import xavier_init, xavier_vec_init, shl, shm
from function import function 
from hopfield_util import symmetric_feedforward_weights, batch_outer, toy_setup, binary_setup, linear_setup


np.random.seed(5)
tf.set_random_seed(5)

weight_factor = 0.1
init = lambda shape, dtype, partition_info: xavier_init(shape[0], shape[1], const = weight_factor)
bias_init = lambda shape, dtype, partition_info: np.zeros((shape[0],))

input_size = 10
hidden_size = 10
output_size = 4
batch_size = 10
epsilon = 0.5
beta = 1.0
epochs = 30
tmp_dir = pj(os.environ["HOME"], "hop")
[os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f.endswith(".png")]

steps_num = 20

net_size = input_size + hidden_size + output_size 
# act = tf.nn.relu
act = lambda x: tf.clip_by_value(x, 0.0, 1.0)
# act = lambda x: x
# act = lambda x: tf.nn.sigmoid(x)4.*x-2.)

x = tf.placeholder(tf.float32, [None, input_size], name="x")
y = tf.placeholder(tf.float32, [None, output_size], name="y")
u = tf.placeholder(tf.float32, [None, net_size], name="u")

cost = lambda xl: 0.5*tf.reduce_sum(tf.square(y - tf.slice(xl, [0, net_size - output_size], [-1, output_size])))
clamp_input = lambda ul: tf.concat_v2([x, tf.slice(ul, [0, input_size], [-1, net_size - input_size])], 1)

W0 = tf.get_variable("W0", [input_size, hidden_size], initializer = init, dtype=tf.float32)
b0 = tf.get_variable("b0", [hidden_size], initializer = bias_init, dtype=tf.float32)

W1 = tf.get_variable("W1", [hidden_size, output_size], initializer = init, dtype=tf.float32)
b1 = tf.get_variable("b1", [output_size], initializer = bias_init, dtype=tf.float32)

biases = tf.expand_dims(tf.concat_v2([tf.zeros(input_size), b0, b1], 0), 1)

W = symmetric_feedforward_weights([W0, W1])
# W = tf.get_variable("W", [net_size, net_size], initializer = init, dtype=tf.float32)


def run_network(u_start, free, beta, epsilon, denergy_stop=1e-07):
	def run_network_callback(step_num, denergy, E_prev, F_prev, u_prev, u_hist, dE_hist):
		p = act(u_prev)

		E = (
			0.5 * tf.reduce_sum(batch_outer(p, p)) 
			- 0.5 * tf.reduce_sum(batch_outer(tf.matmul(p, W), p)) 
			- tf.reduce_sum(tf.matmul(p, biases))
		)

		if free:
			F = E
		else:
			F = E + beta * cost(p)
		dEdu = tf.gradients([F], u_prev)[0]
		
		u_new = u_prev - epsilon * dEdu
		u_new = clamp_input(u_new)
		
		# with tf.control_dependencies([tf.assert_non_negative(F_prev-F)]):
		
		# u_new = tf.Print(u_new, [step_num, F, F_prev-F], message="{} ".format(("Free run" if free else "Clamp run")))
		u_hist = u_hist.write(step_num, u_new)
		dE_hist = dE_hist.write(step_num, dEdu)
		
		return step_num+1, F_prev-F, E, F, u_new, u_hist, dE_hist

  	u_hist = tf.TensorArray(dtype=tf.float32, size=steps_num, tensor_array_name="u_hist")
  	dE_hist = tf.TensorArray(dtype=tf.float32, size=steps_num, tensor_array_name="dE_hist")


	step_num = tf.constant(0, dtype=tf.int32)
	energy = tf.constant(1000, dtype=tf.float32)
	total_energy = tf.constant(1000, dtype=tf.float32)
	denergy = tf.constant(1000, dtype=tf.float32)

	_, _, energy_final, total_energy_final, u_final, u_hist, dE_hist = tf.while_loop(
		body=lambda *args: run_network_callback(*args),
		cond=lambda step_num, denergy, *_: tf.less(step_num, steps_num),
		# cond=lambda step_num, denergy, *_: tf.logical_and(tf.less(step_num, steps_num), tf.greater(denergy, denergy_stop)),
		loop_vars=[step_num, denergy, energy, total_energy, u_start, u_hist, dE_hist]
	)

	return u_final, energy_final, total_energy_final, u_hist.stack(), dE_hist.stack()


u_free, energy_free, _, u_hist0, dE_hist0 = run_network(u, free=True, beta=beta, epsilon=epsilon)

u_clamp, energy_clamp, total_energy_clamp, u_hist1, dE_hist1 = run_network(u_free, free=False, beta=beta, epsilon=epsilon)


dE = (total_energy_clamp - energy_free)/beta

# opt = tf.train.AdamOptimizer(0.01)
# opt = tf.train.RMSPropOptimizer(0.001)
opt = tf.train.GradientDescentOptimizer(0.05)

tvars = tf.trainable_variables()
grads = tf.gradients(dE, tvars)
train_step = opt.apply_gradients(zip(grads, tvars))

p_final = act(u_clamp)
cost_value = cost(p_final)


###########################################################
# x_v, y_v = linear_setup(input_size, output_size, batch_size)
# x_v, y_v = binary_setup(input_size, output_size, batch_size)
x_v, y_v = toy_setup(input_size, output_size, batch_size)

def clamp_input_np(x):
	x[:, :input_size] = x_v
	return x

u_v = np.zeros((batch_size, net_size))
# u_v = np.random.rand(batch_size, net_size)

sess = tf.Session()
sess.run(tf.global_variables_initializer())

# W0 = sess.run(W)
# W1 = W0 * 1.0/np.max(np.abs(np.linalg.eig(W0)[0]))
# sess.run(tf.assign(W, W1))

for e in xrange(50):
	u0, F0, u1, F1, dE_v, p_v, cost_value_v, grads_v, _, p0_v, p1_v, u_hist0v, dE_hist0v, u_hist1v, dE_hist1v = sess.run(		
		[
			u_free, 
			energy_free, 
			u_clamp, 
			energy_clamp, 
			dE,
			p_final,
			cost_value,
			grads,
			train_step,
			act(u_free),
			act(u_clamp),
			u_hist0,
			dE_hist0,
			u_hist1,
			dE_hist1,
		], 
		feed_dict={u: u_v, y: y_v, x: x_v}
	)
	u_v = u0
	if e % 10 == 0:	
		shm(
			u0[:,input_size:].T, 
			u1[:, input_size:].T, 
			p0_v.T, 
			p1_v.T, 
			(u0[:, -output_size:] - y_v).T, 
			file=pj(tmp_dir, "u_{}.png".format(e))
		)
		
	# u_v = clamp_input(u1)
	print "Epoch {}, dE {}, cost {}".format(e, dE_v, cost_value_v)
