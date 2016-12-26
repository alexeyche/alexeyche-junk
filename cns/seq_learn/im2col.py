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
filters_num = 15
L = 24
strides = 1
seq_size = 50

input = tf.placeholder(tf.float32, shape=(batch_size, seq_size, 1, 1), name="Input")

v = np.asarray([np.exp(-np.linspace(0.0, 1.0, L)/0.5)]*filters_num)
v[0,:] = 0.0
# init = lambda shape, dtype, partition_info: v.T.reshape(shape)
filter = vs.get_variable("F", [L, 1, 1, filters_num], initializer = tf.uniform_unit_scaling_initializer(factor=0.1))  

y_t = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='SAME')
y_t = tf.squeeze(y_t, [2])
y_t = tf.transpose(y_t, [1, 0, 2])


input_t = tf.squeeze(input, [2, 3])
input_t = tf.transpose(input_t, [1, 0])
input_t = tf.pad(input_t, [[L/2-1, L/2+1], [0, 0]])

input_shape = input_t.get_shape()
time_steps = input_shape[0]


input_ta = tf.TensorArray(dtype=y_t.dtype, size=time_steps, tensor_array_name="input", clear_after_read=False)
output_ta = tf.TensorArray(dtype=y_t.dtype, size=time_steps-L, tensor_array_name="output")

input_ta = input_ta.unstack(input_t)

time = array_ops.constant(L/2, dtype=dtypes.int32, name="time")

def _time_step(time, output_ta_t):
    input_ta_t = input_ta.gather(tf.range(time-L/2, time+L/2))
    
    input_ta_t.set_shape([L] + input_shape.as_list()[1:])
    input_ta_t = tf.tile(input_ta_t, [filters_num] + [1]*(len(input_shape)-1))    
    input_ta_t = tf.reshape(input_ta_t, [filters_num, L] + input_shape.as_list()[1:])
    input_ta_t = tf.transpose(input_ta_t, [1, 0] + range(2, 2+len(input_shape.as_list())-1))
    
    output_ta_t = output_ta_t.write(time-L/2, input_ta_t)
    return time + 1, output_ta_t

_, output_final_ta = tf.while_loop(
    cond=lambda time, *_: time < time_steps-L/2,
    body=_time_step,
    loop_vars=(time, output_ta),
    parallel_iterations=1
)


output_final = output_final_ta.stack()
output_final_right = [ tf.gradients([v], [filter])[0] for v in tf.unstack(y_t) ]



sess = tf.Session()

writer = tf.summary.FileWriter("/home/alexeyche/tf", sess.graph)

sess.run(tf.global_variables_initializer())
input_v = np.random.randn(seq_size)
# input_v = np.asarray([0.0] * seq_size)
# input_v[25] = 1.0
# input_v[40] = 1.0

y_t_v, output_final_v, output_final_rv, = sess.run([y_t, output_final, output_final_right], {input: input_v.reshape(1, seq_size, 1, 1)})

# output_final_v = np.squeeze(output_final_v)
# output_final_rv = np.squeeze(output_final_rv)
