#!/usr/bin/env python

import os
import tensorflow as tf
import numpy as np
from tensorflow.python.ops import control_flow_ops
from tensorflow.python.ops.rnn_cell import RNNCell
from tensorflow.python.ops.rnn_cell import MultiRNNCell
from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import rnn
from tensorflow.python.ops import init_ops

from matplotlib import pyplot as plt


# def get_form(dt, sigma):
#     #thetaNeuron = ThetaRNNCell(1, input_weights_init = tf.constant_initializer(np.log(1.0/dt)), recc_weights_init = tf.constant_initializer(0))
#     thetaNeuron = ThetaRNNCell(1, dt = dt, sigma = sigma, input_weights_init = tf.constant_initializer(1.0/dt), recc_weights_init = tf.constant_initializer(0))

#     inputs  = [ tf.placeholder(tf.float32, shape=(1, 1), name="Input_{}".format(idx)) for idx in xrange(12) ]
#     init_state = tf.zeros((1, 1))
#     with tf.variable_scope("GenFrom", reuse=None) as scope:
#         outputs, state = rnn.rnn(thetaNeuron, inputs, init_state, scope=scope)

#     init = tf.initialize_all_variables()
#     sess = tf.Session()
#     data = [ np.asarray([[3.0/2.0]]) ] + [ np.zeros((1, 1)) for _ in xrange(len(inputs)-1) ]

#     with tf.device("/cpu:0"):
#         sess.run(init)
#         feed_dict = {k: v for k, v in zip(inputs, data)}
#         outputs_v, _ = sess.run([outputs, state], feed_dict)
#     signal_form = np.asarray(outputs_v)[:,0,0]
#     return signal_form[np.where(signal_form > 1e-08)]







# weights, recc_weights = [], []
# with tf.device("/cpu:0"):
#     sess.run(init)
#     for e in xrange(epochs):
#         elrate = lrate * (decay_rate ** e)
#         print "Epoch {}, learning rate {}".format(e, elrate),
#         sess.run(tf.assign(lr, elrate))

#         feed_dict = {k: v for k, v in zip(inputs, data)}
#         feed_dict[init_state] = np.zeros((batch_size, net_size))
#         feed_dict.update({k: v for k, v in zip(targets, target_v)})

#         fetch = [outputs, state]
#         fetch += [thetaNeuron.W]
#         fetch += [thetaNeuron.U]
#         fetch += [loss, train_step]
#         fetch += grads_raw[-3:]

#         out = sess.run(fetch, feed_dict)
#         outputs_v, state_v = np.asarray(out[0]), out[1]
#         weights.append(out[2])
#         recc_weights.append(out[3])

#         loss_v, train_step_v = out[4], out[5]
#         gW, gU, gB = out[6], out[7], out[8]
#         print ", train loss {}".format(loss_v)
#         if e % 50 == 0 or e == epochs-1:
#             plt.plot(outputs_v[:,0,0]); plt.show()
#             # out_one = outputs_v[:,0,0]
#             # out = outputs_v[:,0, :]

#             # plt.figure(1)
#             # plt.subplot(2,1,1)
#             # plt.imshow(target_seq.T)
#             # plt.subplot(2,1,2)
#             # plt.imshow(out.T)
#             # plt.show()

