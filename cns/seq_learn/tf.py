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

from util import sm, sl, smooth_matrix, smooth, moving_average, norm, outer
from hist_rnn import rnn_with_hist_loop_fn, cat_hist, RnnHistInputTuple

np.random.seed(10)
tf.set_random_seed(10)


seq_size = 200

weight_init_factor = 1.0
dt = 1.0/1000.0
dt_ms = dt * 1000.0

lrate = 1e-02

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
_GLMStateTuple = collections.namedtuple("GLMStateTuple", ("u", "s", "r", "spikes", "spike_history", "dW", "dF", "dR"))
_GLMOutputTuple = collections.namedtuple("GLMOutputTuple", ("spikes", "a", "output", "loss"))


class GLMStateTuple(_GLMStateTuple):
    __slots__ = ()

class GLMOutputTuple(_GLMOutputTuple):
    __slots__ = ()


def safe_log(v):
    return tf.log(tf.maximum(v, 1e-08))

def exp_act(u, u_rest=-2.0, gain=1.0):
    return tf.exp(u_rest + u) * gain

def sigmoid_act(u, u_rest=-5.0):
    return tf.sigmoid(u_rest + u)

def log_exp_act(u, u_rest=-5.0, slope=1.0):
    return tf.log(1.0 + tf.exp((u_rest + u)/slope)) 


def outer(left_v, right_v):
    return mo.matmul(tf.expand_dims(left_v, 1), tf.expand_dims(right_v, 0))


class GLMCell(rc.RNNCell):
    def __init__(self, filters_num, hidden_size, visible_size, filters, activation = sigmoid_act):
        self._activation = activation
        self._num_units = visible_size + hidden_size
        self._filters_num = filters_num
        self._visible_size = visible_size
        self._hidden_size = hidden_size
        self._filters = filters

        self.W = None
        self.F = None
        self.R = None
        self._state_is_tuple = True
        
    @property
    def state_size(self):
        return GLMStateTuple(
            self._num_units, 
            self._num_units, 
            self._num_units, 
            self._num_units, 
            self._num_units, 
            self._num_units,
            self._num_units,
            self._num_units,
        )

    @property
    def output_size(self):
        return GLMOutputTuple(self._num_units, self._num_units, 1, 1)


    def _init_parameters(self):
        if self.W is None:
            self.W = vs.get_variable("W", [self._filters_num + self._num_units, self._num_units], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.F is None:
            self.F = vs.get_variable("F", [L, filters_num], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor))
        if self.R is None:
            self.R = vs.get_variable("R", [L, 1], initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor*0.5))


    def slice(self, t):
        return (
            tf.slice(t, [0, 0], [batch_size, self._hidden_size]),  
            tf.slice(t, [0, self._hidden_size], [batch_size, self._visible_size])
        )

    def __call__(self, input_tuple, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            self._init_parameters()
        
            _input, target = input_tuple

            assert _input.get_shape()[2] == 1, "Need input depth == 1" 
            _input = tf.squeeze(_input, 2)
            
            u, s, r, inner_spikes, spike_history, dW, dF, dR = state
            
            _input_filtered = tf.matmul(_input, self.F)

            s = (1.0 - tau_syn) * s + tf.concat_v2([_input_filtered, inner_spikes], 1)
            u = (1.0 - tau_mem) * u + mo.matmul(s, self.W)
            r = (1.0 - tau_refr) * r
            

            act_raw = self._activation(u)
            act = act_raw * tf.exp(-r)
            
            spikes = tf.where(
                act > tf.random_uniform([batch_size, self._num_units]),
                tf.ones([batch_size, self._num_units]),
                tf.zeros([batch_size, self._num_units])
            )   

            r += spikes * amp_refr
            
            spike_history = cat_hist(spike_history, spikes, 1)

            act_grad = tf.gradients([act], [u])[0]
            
            neuron_derivative = (act_grad/act_raw) * (spikes - act) 

            spike_history_reshaped = tf.reshape(tf.transpose(spike_history, [0, 2, 1]), [batch_size * self._num_units, L])
            
            output_point = tf.matmul(spike_history_reshaped, self.R)
            output_point = tf.reshape(output_point, [batch_size, self._num_units])
            output_point = tf.reduce_sum(output_point, 1)
            output_point = tf.expand_dims(output_point, 1)
            
            loss = tf.reduce_sum(tf.square(output_point - target), 1)/2.0
            loss = tf.expand_dims(loss, 1)
            
            dR += - lrate * tf.gradients([loss], [self.R])[0]
            
            sp = - tf.gradients([loss], [spike_history])[0]
            sp = tf.reduce_sum(sp, [1,2])
            sp = tf.expand_dims(sp, 1)
            
            dW += lrate * outer(
                tf.reduce_sum(s, 0),
                tf.reduce_sum(neuron_derivative * sp, 0)
            )
            

            return GLMOutputTuple(spikes, act, output_point, loss), GLMStateTuple(u, s, r, spikes, spike_history, dW, dF, dR)


input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, 1), name="Input")
target = tf.placeholder(tf.float32, shape=(seq_size, batch_size, 1), name="Target")
sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)

input_n = tf.nn.l2_normalize(input, 0)
target_n = tf.nn.l2_normalize(target, 0)

cell = GLMCell(filters_num, hidden_size, visible_size, filter)

net_input_tuple = RnnHistInputTuple(input_n, target)

state = GLMStateTuple(
    tf.placeholder(tf.float32, [batch_size, net_size], name="u"),
    tf.placeholder(tf.float32, [batch_size, filters_num + net_size], name="s"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="r"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="spikes"),
    tf.placeholder(tf.float32, [batch_size, L, net_size], name="spike_history"),
    tf.placeholder(tf.float32, [batch_size, filters_num + net_size, net_size], name="dW"),
    tf.placeholder(tf.float32, [batch_size, L, filters_num], name="dF"),
    tf.placeholder(tf.float32, [batch_size, L, 1], name="dR"),
)

net_out, finstate, _ = tf.nn.raw_rnn(cell, rnn_with_hist_loop_fn(input_n, target_n, sequence_length, state, L))

spikes_ta, a_ta, output_ta, loss_ta = net_out
spikes = spikes_ta.stack()
a = a_ta.stack()
output = output_ta.stack()
loss = loss_ta.stack()


# l2_loss = tf.nn.l2_loss(input_n - result)


## RUNNING

inputs_v =  moving_average(np.random.randn(seq_size), 10).reshape(seq_size, batch_size, 1)

sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("glm_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())


reward_v, reward_mean_v = None, None
tmp_dir = pj(os.environ["HOME"], "tmp")
[ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]

epochs = 1000
for e in xrange(epochs):
    state_v = GLMStateTuple(
        np.zeros((batch_size, net_size)),
        np.zeros((batch_size, filters_num + net_size)),
        np.zeros((batch_size, net_size)),
        np.zeros((batch_size, net_size)),
        np.zeros((batch_size, L, net_size)),
        np.zeros((batch_size, filters_num + net_size, net_size)),
        np.zeros((batch_size, L, filters_num)),
        np.zeros((batch_size, L, 1)),
    )

    sess_out = sess.run(
        [
            a,
            spikes,
            finstate,
            input_n,
            target_n,
            output,
            loss
            # l2_loss,
        ],
        {
            input: inputs_v,
            target: inputs_v,
            state: state_v,
            sequence_length: np.asarray([seq_size]*batch_size)
        }
    )

    a_v, spikes_v, finstate_v = sess_out[0:3]
    input_n_v, target_n_v, output_v, loss_v = sess_out[3:10]
    
    if hidden_size > 0:
        output_smooth = smooth_matrix(np.squeeze(spikes_v[:, :, hidden_size:]))
    else:
        output_smooth = smooth_matrix(np.squeeze(spikes_v))
    
    # loss = np.mean(np.square(output_smooth - target_smooth))
    dW = np.mean(finstate_v.dW, 0)
    dF = np.mean(finstate_v.dF, 0)
    dR = np.mean(finstate_v.dR, 0)
    assert not np.any(np.isnan(dW)), "Found nan in dW"
    assert not np.any(np.isnan(dF)), "Found nan in dF"
    assert not np.any(np.isnan(dR)), "Found nan in dR"

    F, W, R = sess.run([cell.F, cell.W, cell.R])
    
    # dF = dF.reshape((L, 1, 1, filters_num))

    sess.run(cell.R.assign(cell.R + 0.1*dR))
    sess.run(cell.W.assign(cell.W + dW))
    
    # new_W, new_F = sess.run([cell.W, filter])
    sl(output_v, input_n_v, (output_v-input_n_v) ** 2, file=pj(os.environ["HOME"], "tmp", "{}.png".format(e)))
    sm(W, file=pj(os.environ["HOME"], "tmp", "W_{}.png".format(e)))
    sl(R, file=pj(os.environ["HOME"], "tmp", "R_{}.png".format(e)))
    sm(spikes_v, file=pj(os.environ["HOME"], "tmp", "sp_{}.png".format(e)))
    print "Epoch {}, loss {}".format(e, np.mean(loss_v))