#!/usr/bin/env python

import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj

from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell as rc

import librosa as lr
import os
import scipy

from env import current as env
from util import norm
from model import ThetaRNNCell


def generate_dct_dictionary(l, size):
    p = np.asarray(xrange(l))
    filters = np.zeros((l, size))
    for fi in xrange(size):
        filters[:, fi] = np.cos((np.pi * (2 * fi + 1) * p)/(2*l))
        filters[0, fi] *= 1.0/np.sqrt(2.0)
        # filters[fi, 1:] *= np.sqrt(2/l) 
    return filters * np.sqrt(2.0/l)

def sigmoid(x): 
    return 1.0/(1.0 +np.exp(-x))


class Config(object):
    def __init__(self, **kwargs):
        self.seq_size = kwargs.get("seq_size", 3000)
        self.batch_size = kwargs.get("batch_size", 1)
        self.filter_len = kwargs.get("filter_len", 150)
        self.filters_num = kwargs.get("filters_num", 100)
        self.target_sr = kwargs.get("target_sr", 3000)
        self.strides = kwargs.get("strides", 8)
        self.lrate = kwargs.get("lrate", 1e-04)
        self.net_size = kwargs.get("net_size", 200)
        self.num_of_layers = kwargs.get("num_of_layers", 2)
        self.forecast_ms = kwargs.get("forecast_ms", 200)

c = Config()
epochs = 1000

# cells = ThetaRNNCell(c.net_size) 
# net_out_size = c.net_size

# cells = rc.LSTMCell(c.net_size) 
# cells = rc.GRUCell(c.net_size)

# cell_type = rc.BasicRNNCell
cell_type = ThetaRNNCell

cells = rc.MultiRNNCell([cell_type(c.net_size) for _ in xrange(c.num_of_layers)])
net_out_size = cells._cells[-1].state_size

forecast_steps = int(c.target_sr*c.forecast_ms/1000.0)
data_size = c.seq_size * 10

#init = lambda shape, dtype: np.reshape(-np.sqrt(3) / np.sqrt(shape[0]) + np.random.random((shape[0], shape[3])) * 2.0*np.sqrt(3) / np.sqrt(shape[0]), (shape[0], 1, 1, shape[3]))       
init = lambda shape, dtype: generate_dct_dictionary(shape[0], shape[3]).reshape(shape[0], 1, 1, shape[3])


input = tf.placeholder(tf.float32, shape=(1, c.seq_size, 1, 1), name="Input")
target = tf.placeholder(tf.float32, shape=(1, c.seq_size, 1, 1), name="Target")

filter = vs.get_variable("W", [c.filter_len, 1, 1, c.filters_num], initializer=init)            
recov_filter = vs.get_variable("Wr", [c.filter_len, 1, 1, net_out_size], initializer=init)

state = tf.placeholder(tf.float32, shape=(c.batch_size, cells.state_size), name="State")

conv_out = tf.nn.conv2d(input, filter, strides=[1, c.strides, 1, 1], padding='VALID')
conv_out = tf.squeeze(conv_out, squeeze_dims=[0])

net_out, finstate = rnn.dynamic_rnn(cells, conv_out, initial_state = state, time_major=True)
net_out = tf.expand_dims(net_out, 0)

output = tf.nn.conv2d_transpose(net_out, recov_filter, output_shape = (1, c.seq_size, 1, 1), strides=[1, c.strides, 1, 1], padding='VALID')

cost = tf.nn.l2_loss(output - target) / c.seq_size / c.batch_size

optimizer = tf.train.AdamOptimizer(c.lrate)
# optimizer = tf.train.RMSPropOptimizer(c.lrate)
# optimizer = tf.train.AdagradOptimizer(c.lrate)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(cost, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

apply_grads = optimizer.apply_gradients(zip(grads, tvars))


fname = env.dataset(os.listdir(env.dataset())[0])
df = env.run("test_data.pkl")

if not os.path.exists(df):
    song_data_raw, source_sr = lr.load(fname)
    print "Got sampling rate {}, resampling to {} ...".format(source_sr, c.target_sr)
    song_data = lr.resample(song_data_raw, source_sr, c.target_sr, scale=True)
    song_data = song_data[:data_size,]

    np.save(open(df, "w"), song_data)
else:
    song_data = np.load(open(df))

data, data_denom = norm(song_data)


sess = tf.Session()
sess.run(tf.initialize_all_variables())


def run(state_v, inputs_v, targets_v):
    return sess.run(
        [
	        output,
	        finstate, 
	        cost, 
	        net_out,
	        conv_out,
	        filter, 
	        recov_filter,
	        apply_grads
    	], 
        {
            input: inputs_v.reshape(1, c.seq_size, 1, 1),
            target: targets_v.reshape(1, c.seq_size, 1, 1),
            state: state_v,
        }
    )

def eval(state_v, inputs_v):
    return sess.run(
        [
        	output,
        	finstate, 
    	], 
        {
            input: inputs_v.reshape(1, c.seq_size, 1, 1),
            state: state_v,
        }
    )

def generate(start_id, steps = 20):
    state_v = np.zeros((c.batch_size, cells.state_size))

    inputs_v, state_v = eval(state_v, data[start_id:(start_id+c.seq_size)])

    generated = [inputs_v.reshape(c.seq_size)]

    for step_id in xrange(steps):
        inputs_v, state_v = eval(state_v, inputs_v)
        generated.append(inputs_v.reshape(c.seq_size)[-forecast_steps:,])

    return np.concatenate(generated)


for epoch in xrange(epochs):
    state_v = np.zeros((c.batch_size, cells.state_size))
    
    p_sub = sigmoid(-6.0+12*epoch/float(epochs))
    
    start_ids = np.arange(0, data.shape[0]-c.seq_size, forecast_steps if forecast_steps > 0 else c.seq_size)

    losses = []
    
    out_v = []
    
    for start_id in start_ids:
        fstart_id = start_id + forecast_steps
        
        inputs_v = data[start_id:(start_id+c.seq_size)] 
        targets_v = data[fstart_id:(fstart_id+c.seq_size)]

        if forecast_steps > 0 and out_v and p_sub > np.random.random_sample():
            inputs_v = out_v[-1]
    
        ret = run(state_v, inputs_v, targets_v)
        
        curr_out_v, state_v, cost_v = ret[0:3]
        
        out_v.append(curr_out_v.reshape(c.seq_size))
        
        losses.append(cost_v)
        
    print "Epoch {}, loss {}".format(epoch, sum(losses)/len(losses))
    if epoch % 10 == 0 or epoch == epochs - 1:
        if forecast_steps > 0:
        	gen_out = generate(0)
    	else:
    		gen_out = np.concatenate(out_v)

        gen_out *= data_denom
        waveform_resampled = lr.resample(gen_out, c.target_sr, 22050, scale=True)

        dst = env.run("test_data_{}.wav".format(epoch))

        print "Saving waveform as {}".format(dst)
        lr.output.write_wav(dst, waveform_resampled, 22050)
