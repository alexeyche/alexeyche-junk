#!/usr/bin/env python

import os
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj

from tensorflow.python.ops import rnn_cell as rc
from tensorflow.python.ops import rnn

from util import xavier_init, xavier_vec_init
from env import Env
from util import shs, shm, shl
from multi_rnn_cell import MultiRNNCellFull

def safe_log(v):
    return tf.log(tf.maximum(v, 1e-08))

def exp_act(u, u_rest=-10.0, gain=1000.0):
    return (tf.exp(u_rest + u) - tf.exp(u_rest)) * gain

def sigmoid_act(u, u_rest=-5.0):
    return tf.sigmoid(u_rest + u)

def log_exp_act(u, u_rest=-10.0, slope=1.0):
    return tf.log(1.0 + tf.exp((u_rest + u)/slope)) 

def outer(left_v, right_v):
    return tf.matmul(tf.expand_dims(left_v, 1), tf.expand_dims(right_v, 0))




class GLMCell(rc.RNNCell):
    class Params(object):
        def __init__(self, input_size, num_units, weight_factor=1.0):
            self.W = tf.get_variable(
                "W", 
                [input_size, num_units], 
                initializer=lambda shape, dtype, partition_info: xavier_init(shape[0], shape[1], const=weight_factor)
            )

            self.Wr = tf.get_variable(
                "Wr", 
                [num_units, num_units], 
                initializer=lambda shape, dtype, partition_info: xavier_init(shape[0], shape[1], const=weight_factor)
            )


    def __init__(self, num_units, activation = exp_act):
        self._activation = activation
        self._num_units = num_units

        self.p = None
        self._state_is_tuple = True
        
    @property
    def state_size(self):
        return (
            self._num_units,
            self._num_units,
            self._num_units,
            self._num_units,
            self._num_units,
            (self._num_units, self._num_units,),
            (self._num_units, self._num_units,),
        )

    @property
    def output_size(self):
        return (
            self._num_units,
            self._num_units,
            self._num_units,
            self._num_units,
            self._num_units,
        )


    def __call__(self, _input, state, scope=None):
        if isinstance(_input, tuple):
            _input = _input[0]

        if self.p is None:
            input_shape = _input.get_shape()
            self.p = GLMCell.Params(input_shape[-1], self._num_units)

        with tf.variable_scope(scope or type(self).__name__):
            
            u, r, inner_spikes, pre_stdp, post_stdp, dW, dWr = state
            
            u = (1.0 - tau_mem) * u + tf.matmul(_input, self.p.W) + tf.matmul(inner_spikes, self.p.Wr)
            r = (1.0 - tau_refr) * r

            act_raw = self._activation(u)
            act = act_raw * tf.exp(-r)

            spikes = tf.where(
                act > 0.01, # tf.random_uniform([batch_size, self._num_units]),
                tf.ones([batch_size, self._num_units]),
                tf.zeros([batch_size, self._num_units])
            )   
                        
            r += spikes * amp_refr
                        
            pre_stdp = (1.0 - tau_plus) * pre_stdp + _input
            post_stdp = (1.0 - tau_minus) * post_stdp + spikes
            

            dW += lrate * (outer(tf.reduce_mean(pre_stdp, 0), tf.reduce_mean(spikes, 0)) \
                - 1.1*outer(tf.reduce_mean(_input, 0), tf.reduce_mean(post_stdp, 0)))
            
            dWr += lrate * (outer(tf.reduce_mean(post_stdp, 0), tf.reduce_mean(spikes, 0)) \
                - 1.1*outer(tf.reduce_mean(spikes, 0), tf.reduce_mean(post_stdp, 0)))
            
            output = (spikes, u, act, pre_stdp, post_stdp)
            output_state = (u, r, spikes, pre_stdp, post_stdp, dW, dWr)
            
            return output, output_state


dt = 1.0/1000.0
dt_ms = dt * 1000.0

lrate = 0.01 #1e-03

batch_size = 1
input_size = 100
net_size = 100
seq_size = 100

tau_syn = dt_ms/10.0
tau_mem = dt_ms/10.0
tau_refr = dt_ms/2.0
tau_learn = dt_ms/10.0
tau_plus = dt_ms/10.0
tau_minus = dt_ms/10.0

amp_refr = 100.0
layers_num = 1

epochs = 100

cell = MultiRNNCellFull([GLMCell(net_size) for _ in xrange(layers_num)])

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")

state = tuple( (
    tf.placeholder(tf.float32, [batch_size, net_size], name="u"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="r"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="spikes"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="pre_stdp"),
    tf.placeholder(tf.float32, [batch_size, net_size], name="post_stdp"),
    tf.placeholder(tf.float32, [net_size, net_size], name="dW"),
    tf.placeholder(tf.float32, [net_size, net_size], name="dWr"),
) for _ in xrange(layers_num) )


spikes, finstate = rnn.dynamic_rnn(cell, input, initial_state=state, time_major=True)


inputs_v = np.zeros((seq_size, batch_size, input_size))

# x_rate = 2.0
# for bi in xrange(batch_size):
#     for tt in xrange(seq_size):
#         for ni in xrange(input_size):
#             if np.random.random() <= x_rate/seq_size:
#                 inputs_v[tt, bi, ni] = 1.0

for i in xrange(seq_size):
    if i % 10 == 0:
        inputs_v[i, 0, i] = 1.0

# inputs_v[50, 0, 50] = 1.0

state_v = tuple( (
    np.zeros((batch_size, net_size)),
    np.zeros((batch_size, net_size)),
    np.zeros((batch_size, net_size)),
    np.zeros((batch_size, net_size)),
    np.zeros((batch_size, net_size)),
    np.zeros((net_size, net_size)),
    np.zeros((net_size, net_size)),
) for _ in xrange(layers_num) )

sess = tf.Session()
saver = tf.train.Saver()


env = Env("simple_test", clear_pics=True)

model_fname = env.run("glm_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())


epochs = 100
ww, wr = [], []
for e in xrange(epochs):
    out = sess.run(
        [
            spikes,
            finstate,
        ],
        {
            input: inputs_v,
            state: state_v,
        }
    )

    spikes_v = out[0]
    finstate_v = out[1]

    W_vals, Wr_vals = [], []
    for c, s in zip(cell._cells, finstate_v):
        dW, dWr = s[-2], s[-1]
        W = sess.run(c.p.W)
        sess.run(c.p.W.assign(c.p.W + dW))

        Wr = sess.run(c.p.Wr)
        sess.run(c.p.Wr.assign(c.p.Wr + dWr))

        W_vals.append(W+dW)
        Wr_vals.append(Wr+dWr)
    
    ww.append(W_vals)
    wr.append(Wr_vals)


    osize = len(cell._cells[0].output_size)
    
    shm(*[spikes_v[li*osize] for li in xrange(layers_num)], file=env.run("spikes_{}.png".format(e)))
    

    shm(*W_vals, file=env.run("W_{}.png".format(e)))
    shm(*Wr_vals, file=env.run("Wr_{}.png".format(e)))
    print "Epoch {}".format(e)

ww = np.concatenate(ww)
wr = np.concatenate(wr)
for syn_i in  xrange(input_size):
    shl(ww[:,syn_i,:], file=env.run("w_hist", "syn_{}.png".format(syn_i)))

for ni in  xrange(net_size):
    shl(wr[:,ni,:], file=env.run("wr_hist", "n_{}.png".format(ni)))


# pre_stdp = np.squeeze(spikes_v[3])
# pre_spikes = np.squeeze(inputs_v)
# post_stdp = np.squeeze(spikes_v[4])
# post_spikes = np.squeeze(spikes_v[0])

# dw = np.zeros(W.shape)
# for ti in xrange(seq_size):
#     # for ni in xrange(net_size):
#     #     for syn_i in xrange(input_size):
#     #         dw[syn_i, ni] += pre_stdp[ti, syn_i] * post_spikes[ti, ni] - post_stdp[ti, ni] * pre_spikes[ti, syn_i]


#     dw += np.outer(pre_stdp[ti,:] , post_spikes[ti,:]) - np.outer(pre_spikes[ti,:], post_stdp[ti,:])
