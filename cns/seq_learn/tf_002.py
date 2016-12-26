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

def sm(matrix):
    plt.imshow(np.squeeze(matrix).T)
    plt.colorbar()
    plt.show()

def smooth(signal, sigma=0.01, filter_size=50):
    lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
    return np.convolve(lf_filter, signal, mode="same")

def smooth_matrix(m, sigma=0.01, filter_size=50):
    res = np.zeros(m.shape)
    for dim_idx in xrange(m.shape[1]):
        res[:, dim_idx] = smooth(m[:, dim_idx], sigma, filter_size)
    return res


def im2col(seq, filters_num, L):
    assert len(seq.get_shape()) == 2
    
    input_t = tf.pad(seq, [[L/2-1, L/2+1], [0, 0]])
    input_shape = input_t.get_shape()
    time_steps = input_shape[0]


    input_ta = tf.TensorArray(dtype=seq.dtype, size=time_steps, tensor_array_name="input", clear_after_read=False)
    output_ta = tf.TensorArray(dtype=seq.dtype, size=time_steps-L, tensor_array_name="output")

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
    output_final.set_shape([time_steps-L] + output_final.get_shape().as_list()[1:])
    return tf.transpose(output_final, [0, 3, 1, 2])


np.random.seed(10)
tf.set_random_seed(10)


seq_size = 100

weight_init_factor = 0.1
dt = 1.0/1000.0
dt_ms = dt * 1000.0

lrate = 1e-03

batch_size = 1
filters_num = 90
L = 100
strides = 1
hidden_size = 0
visible_size = 10
net_size = hidden_size + visible_size

tau_syn = dt_ms/10.0
tau_mem = dt_ms/10.0
tau_refr = dt_ms/2.0
tau_learn = dt_ms/10.0
tau_long = dt_ms/float(seq_size)
amp_refr = 100.0

env = Env("simple_test")
_GLMInputTuple = collections.namedtuple("GLMInputTuple", ("input", "target", "dInput_dF"))
_GLMStateTuple = collections.namedtuple("GLMStateTuple", ("u", "s", "r", "spikes", "dW", "dF", "reward", "reward_mean"))
_GLMOutputTuple = collections.namedtuple("GLMOutputTuple", ("spikes", "a", "factor", "reward", "reward_mean"))

class GLMInputTuple(_GLMInputTuple):
    __slots__ = ()


class GLMStateTuple(_GLMStateTuple):
    __slots__ = ()

class GLMOutputTuple(_GLMOutputTuple):
    __slots__ = ()


def safe_log(v):
    return tf.log(tf.maximum(v, 1e-08))

def exp_act(u, u_rest=-5.0, gain=1.0):
    return tf.exp(u_rest + u) * gain

def sigmoid_act(u, u_rest=-5.0):
    return tf.sigmoid(u_rest + u)

def log_exp_act(u, u_rest=-5.0, slope=1.0):
    return tf.log(1.0 + tf.exp((u_rest + u)/slope)) 




class GLMCell(rc.RNNCell):
    def __init__(self, filters_num, hidden_size, visible_size, filters, activation = exp_act):
        self._activation = activation
        self._num_units = visible_size + hidden_size
        self._filters_num = filters_num
        self._visible_size = visible_size
        self._hidden_size = hidden_size
        self._filters = filters

        self.W = None
        self._state_is_tuple = True
        
    @property
    def state_size(self):
        return GLMStateTuple(
            self._num_units, 
            self._filters_num + self._num_units, 
            self._num_units, 
            self._num_units, 
            (self._filters_num, self._num_units),
            (L, self._filters_num),
            self._num_units,
            self._num_units
        )

    @property
    def output_size(self):
        return GLMOutputTuple(self._num_units, self._num_units, self._num_units, 1, 1)


    def _init_parameters(self):
        if self.W is None:
            self.W = vs.get_variable("W", [self._filters_num + self._num_units, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        

    def slice(self, t):
        return (
            tf.slice(t, [0, 0], [batch_size, self._hidden_size]),  
            tf.slice(t, [0, self._hidden_size], [batch_size, self._visible_size])
        )

    def __call__(self, input_tuple, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            self._init_parameters()

            _input, target, dInput_dF = input_tuple
            
            u, s, r, inner_spikes, dW, dF, reward, reward_mean = state
            
            s = (1.0 - tau_syn) * s + tf.concat_v2([_input, inner_spikes], 1)
            u = (1.0 - tau_mem) * u + mo.matmul(s, self.W)
            r = (1.0 - tau_refr) * r
            

            act_raw = self._activation(u)
            act = act_raw * tf.exp(-r)
            
            spikes = tf.where(
                act > tf.random_uniform([batch_size, self._num_units]),
                tf.ones([batch_size, self._num_units]),
                tf.zeros([batch_size, self._num_units])
            )   
            
            hidden_spikes, _ = self.slice(spikes)
            _, act_visible = self.slice(act)
            
            
            reward_mean = (1.0 - tau_long) * reward_mean + tau_long * reward
            reward = (1.0 - tau_learn) * reward + tau_learn * tf.reduce_sum(
                target * safe_log(act_visible*dt) + (1.0 - target) * safe_log(1.0 - act_visible*dt)
            , 1)
            
            r += spikes * amp_refr

            act_grad = tf.gradients([act], [u])[0]
            
            learn_target = tf.concat_v2([hidden_spikes, target], 1)

            factor = tf.concat_v2([
                (reward - reward_mean) * tf.ones((batch_size, hidden_size,)), 
                tf.ones((batch_size, visible_size,))
            ], 1)
            
            neuron_derivative = tf.reduce_sum( (act_grad/act_raw) * (learn_target - act) * factor, 0)
            

            Wsliced = tf.slice(self.W, [0, 0], [self._filters_num, self._num_units])
            
            dF_deriv_part = tf.squeeze(mo.matmul(Wsliced, tf.expand_dims(neuron_derivative, 1)))
            
            
            dW += lrate * outer(
                tf.reduce_sum(s, 0),
                neuron_derivative
            )
            
            dInput_dF = tf.reduce_mean(dInput_dF, 0)
            
            dF += lrate * dF_deriv_part * dInput_dF
            
            return GLMOutputTuple(spikes, act, factor, reward, reward_mean), GLMStateTuple(u, s, r, spikes, dW, dF, reward, reward_mean)


input = tf.placeholder(tf.float32, shape=(batch_size, seq_size, 1, 1), name="Input")
filter = vs.get_variable("F", [L, 1, 1, filters_num], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))            
# bias = vs.get_variable("bias", [filters_num], initializer=lambda shape, dtype: np.zeros(filters_num))

net_input = tf.nn.conv2d(input, filter, strides=[1, strides, 1, 1], padding='SAME')
net_input = tf.squeeze(net_input, [2])
net_input = tf.transpose(net_input, [1, 0, 2]) # making time major


input_t = tf.squeeze(input, [2, 3])
input_t = tf.transpose(input_t, [1, 0])
dInput_dF = im2col(input_t, filters_num, L)

cell = GLMCell(filters_num, hidden_size, visible_size, filter)

target = tf.placeholder(tf.float32, shape=(seq_size, batch_size, visible_size), name="Target")

net_input_tuple = GLMInputTuple(net_input, target, dInput_dF)

state = GLMStateTuple(
    tf.placeholder(tf.float32, [batch_size, net_size], name="u"),
    tf.placeholder(tf.float32, [batch_size, filters_num + net_size], name="s"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="r"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="spikes"),
    tf.placeholder(tf.float32, [filters_num + net_size, net_size], name="dW"),
    tf.placeholder(tf.float32, [L, filters_num], name="dF"),
    tf.placeholder(tf.float32, [batch_size, 1], name="reward"),
    tf.placeholder(tf.float32, [batch_size, 1], name="reward_mean"),
)

net_out, finstate = rnn.dynamic_rnn(cell, net_input_tuple, initial_state=state, time_major=True, parallel_iterations = 1)
spikes, a, factor, reward, reward_mean = net_out

_, a_visible = tf.slice(a, [0, 0, 0], [seq_size, batch_size, hidden_size]), tf.slice(a, [0, 0, hidden_size], [seq_size, batch_size, visible_size])

log_ll = tf.reduce_mean(net_input_tuple.target * safe_log(a_visible*dt) + (1.0 - net_input_tuple.target) * safe_log(1.0 - a_visible*dt))

## RUNNING

inputs_v = np.random.randn(batch_size, seq_size, 1, 1)


targets_v = np.zeros((seq_size, batch_size, visible_size))

for bi in xrange(batch_size):
    for si in xrange(seq_size):
        if si % visible_size == 0:
            targets_v[si, bi, int((float(si)/seq_size) * visible_size)] = 1.0

# targets_v[seq_size/2, 0, 2] = 1.0

sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("glm_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())


target_smooth = smooth_matrix(np.squeeze(targets_v))

reward_v, reward_mean_v = None, None

epochs = 200
for e in xrange(epochs):
    state_v = GLMStateTuple(
        np.zeros((batch_size, net_size)),
        np.zeros((batch_size, filters_num + net_size)),
        np.zeros((batch_size, net_size)),
        np.zeros((batch_size, net_size)),
        np.zeros((filters_num + net_size, net_size)),
        np.zeros((L, filters_num)),
        (reward_v[-1,:,:] if not reward_v is None else np.zeros((batch_size, 1))),
        (reward_mean_v[-1,:,:] if not reward_mean_v is None else np.zeros((batch_size, 1))),
    )

    sess_out = sess.run(
        [
            net_input,
            a,
            spikes,
            finstate,
            log_ll,
            factor,
            reward,
            reward_mean
        ],
        {
            input: inputs_v,
            target: targets_v,
            state: state_v,
        }
    )
    net_input_v, a_v, spikes_v, finstate_v, log_ll_v, factor_v, reward_v, reward_mean_v = sess_out
    
    if hidden_size > 0:
        output_smooth = smooth_matrix(np.squeeze(spikes_v[:, :, hidden_size:]))
    else:
        output_smooth = smooth_matrix(np.squeeze(spikes_v))
    
    loss = np.mean(np.square(output_smooth - target_smooth))
    dW = finstate_v.dW
    dF = finstate_v.dF
    assert not np.any(np.isnan(dW)), "Found nan in dW"
    assert not np.any(np.isnan(dF)), "Found nan in dF"

    W, F = sess.run([cell.W, filter])
    
    dF = dF.reshape((L, 1, 1, filters_num))

    sess.run(filter.assign(filter + dF))
    sess.run(cell.W.assign(cell.W + dW))
    
    new_W, new_F = sess.run([cell.W, filter])

    print "Epoch {}, log ll {}, loss {}".format(e, log_ll_v, loss)