#!/usr/bin/env python

import os
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob
from env import Env

import collections

from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell as rc
from tensorflow.python.ops import math_ops as mo
from tensorflow.python.ops import array_ops
from tensorflow.python.framework import dtypes

batch_size = 1
filters_num = 9
L = 25
strides = 1
seq_size = 50

input = tf.placeholder(tf.float32, shape=(batch_size, seq_size, 1, 1), name="Input")
init = lambda shape, dtype, partition_info: np.asarray([np.exp(-np.linspace(0.0, 1.0, L)/0.5)]*filters_num).T.reshape(shape)
filter = vs.get_variable("F", [L, 1, 1, filters_num], initializer = init)  

y_t = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='SAME')
dy_t = tf.nn.conv2d(y_t, tf.reshape(filter, [L, 1, filters_num, 1]), strides=[1, strides, 1, 1], padding='SAME')

y_t = tf.transpose(y_t, [1, 0, 2, 3])

# input_pad = tf.pad(input, tf.constant([[0, 0], [L, L], [0, 0], [0,0]]))
# y_t_pad = tf.pad(y_t, tf.constant([[0, 0], [L, L], [0, 0], [0,0]]))

# print y_t_pad.get_shape()

# print input_pad.get_shape()



x_t = filter


input_t = tf.transpose(input, [1, 0, 2, 3])
input_shape = input_t.get_shape()
time_steps = input_shape[0]

input_ta = tf.TensorArray(dtype=y_t.dtype, size=time_steps, tensor_array_name="input")
output_ta = tf.TensorArray(dtype=y_t.dtype, size=time_steps, tensor_array_name="output")

input_ta = input_ta.unstack(input_t)

time = array_ops.constant(0, dtype=dtypes.int32, name="time")

def _time_step(time, output_ta_t):
    input_ta_t = input_ta.read(time)
    # grads = tf.gradients([input_ta_t], [x_t])[0]
    # output_ta_t = output_ta_t.write(time, grads)
    output_ta_t = output_ta_t.write(time, input_ta_t)
    return time + 1, output_ta_t

_, output_final_ta = tf.while_loop(
    cond=lambda time, *_: time < time_steps,
    body=_time_step,
    loop_vars=(time, output_ta),
    parallel_iterations=1
)


g = tf.gradients([y_t], [x_t])

# output_final = output_final_ta.stack()
# output_final = output_final_ta.read(0)
# output_final.set_shape([time_steps] + x_t.get_shape().as_list())

# output_final = tf.map_fn(lambda x: tf.gradients([x], [x_t])[0], y_t)

output_final = [ tf.gradients([v], [x_t])[0] for v in tf.unstack(y_t) ]




sess = tf.Session()

writer = tf.summary.FileWriter("/home/alexeyche/tf", sess.graph)

sess.run(tf.global_variables_initializer())
input_v = np.asarray([0.0] * seq_size)
input_v[5] = 1.0
input_v[40] = 1.0

y_t_v, output_final_v, g_v, dy_t_v = sess.run([y_t, output_final, g, dy_t], {input: input_v.reshape(1, seq_size, 1, 1)})

output_final_v = np.squeeze(output_final_v)

plt.figure(1)
plt.imshow(output_final_v)
plt.figure(2)
plt.plot(np.squeeze(g_v))
plt.show()