#!/usr/bin/env python

import tensorflow as tf
import numpy as np
from tensorflow.python.ops import control_flow_ops
from matplotlib import pyplot as plt

tf.reset_default_graph()

dt = 0.01
sigma = 0.01
b = 0.1

eps = tf.placeholder(tf.float32, shape=(), name="eps")
I  = tf.placeholder(tf.float32, shape=(), name="I")
time_steps = tf.placeholder(tf.int32, shape=(), name="time_steps")

theta_init = np.zeros([1], dtype=np.float32)

theta  = tf.Variable(theta_init, name="theta")


init = tf.initialize_all_variables()

sess = tf.Session()

new_theta = theta +  eps * (1.0 - tf.cos(theta) + (1.0 + tf.cos(theta)) * (I - b))
step = theta.assign(new_theta)
act =  tf.exp( - tf.square(-1.0 - tf.cos(theta) )/( 2.0 * sigma ** 2)) 


theta_acc = []
act_acc = []
inp_acc = []

rate = 200.0/10.0

with tf.device("/cpu:0"):
	sess.run(init)
	for _ in xrange(0, 1000):
		inp = 0.0
		if dt * rate >  np.random.random(1)[0]:
			inp = 1.0
		inp *= 1.0
		inp_acc.append(inp)

		v, va = sess.run([step, act], {eps: dt, I: inp})
		theta_acc.append(v)
		act_acc.append(va)
	for _ in xrange(0, 1000):
		inp = 0.0
		inp_acc.append(inp)

		v, va = sess.run([step, act], {eps: dt, I: inp})
		theta_acc.append(v)
		act_acc.append(va)
	
plt.figure(1)
plt.subplot(3,1,1)
plt.plot(np.asarray(theta_acc))
plt.subplot(3,1,2)
plt.plot(np.asarray(inp_acc))
plt.subplot(3,1,3)
plt.plot(np.asarray(act_acc))

plt.show()


sess.close()

