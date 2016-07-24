# -*- coding: utf-8 -*-

import numpy as np
import librosa as lr
import logging
from os.path import expanduser as pp
from matplotlib import pyplot as plt
from os.path import join as pj
import os

import tensorflow as tf
from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import math_ops
from tensorflow.python.ops.math_ops import sigmoid
from tensorflow.python.ops.math_ops import tanh


DEVICE = "gpu"

ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_list_files = []
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".npy"):
    	data_list_files.append(pj(ds_dir, f))
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

   
res_dir = pj(os.environ["HOME"], "Music", "ml", "impro_res")
if not os.path.exists(res_dir):
    os.makedirs(res_dir)

source_id = 1


ms_frame = 10
sr = 22050
hop = int(np.round(ms_frame * sr / 1000.0))


y, sr = lr.load(data_source[source_id])
stft = lr.stft(y, hop_length=hop)
source = np.load(data_list_files[source_id]) 


inp = tf.placeholder(tf.float32, shape=(source.shape[1], source.shape[0]), name="InputFeats")

N = 2000

W1 = vs.get_variable("W1", [source.shape[0], N])
b1 = vs.get_variable("b1", [N])

W2 = vs.get_variable("W2", [N, stft.shape[0]])
b2 = vs.get_variable("b2", [stft.shape[0]])


dst = tf.placeholder(tf.float32, shape=(stft.shape[1], stft.shape[0]), name="Dst")
#dst_imag = tf.placeholder(tf.float32, shape=(stft.shape[1], stft.shape[0]), name="DstImag")

l1 = tf.nn.sigmoid(tf.add(math_ops.matmul(inp, W1), b1))
l2 = tf.add(math_ops.matmul(l1, W2), b2) 

#imag = math_ops.matmul(inp, Wimag)


loss = tf.reduce_sum(tf.pow(dst-l2, 2)) + 0.1 * tf.reduce_sum(W1) + 0.1 * tf.reduce_sum(W2) # + tf.nn.l2_loss(imag-dst_imag)


train_step = tf.train.AdadeltaOptimizer(0.0001).minimize(loss)

with tf.device("/{}:0".format(DEVICE)):
    sess = tf.Session()
    sess.run(tf.initialize_all_variables())
    for e in xrange(1000):
    	
    	loss_v, _ = sess.run(
    		[loss, train_step], 
    		{inp: source.T, dst: stft.real.T} #, dst_imag: stft.imag.T}
    	)
    	
    	print loss_v

y_r = lr.istft(source_r.T, hop_length=hop)
lr.output.write_wav(pj(res_dir, "{}_result.wav".format(source_id)), y_r, sr)
