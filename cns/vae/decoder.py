

from gen_rnn import dynamic_gen_rnn
from gen_rnn import ProjectionCell
import tensorflow as tf
import numpy as np
from util import shm, shl, shs
import os
from os.path import join as pj

np.random.seed(10)
tf.set_random_seed(10)

seq_size = 100
dim_size = 1
net_size = 100
batch_size = 1
lrate = 1e-03

tmp_dir = "/home/{}/tmp".format(os.environ["USER"])
[os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f.endswith(".png")]

epochs = 1000

input = tf.placeholder(tf.float32, shape=(batch_size, dim_size), name="Input")
state = tf.placeholder(tf.float32, shape=(batch_size, net_size), name="State")

outputs_target = tf.placeholder(tf.float32, shape=(seq_size, batch_size, dim_size), name="OutputsTarget")


cell = ProjectionCell(tf.nn.rnn_cell.GRUCell(net_size), dim_size, activation=tf.nn.tanh)

outputs, finstate = dynamic_gen_rnn(cell, input, seq_size, initial_state=state)


loss = tf.nn.l2_loss(outputs - outputs_target)


optimizer = tf.train.AdamOptimizer(lrate)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(loss, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

apply_grads = optimizer.apply_gradients(zip(grads, tvars))



sess = tf.Session()
sess.run(tf.global_variables_initializer())


input_v = np.random.randn(batch_size, dim_size)

target_v = np.sin(np.linspace(0, 20, num=seq_size)).reshape((seq_size, 1, 1))


for e in xrange(epochs):
	outputs_v, finstate_v, loss_v, _ = sess.run(
		[outputs, finstate, loss, apply_grads], 
		{
			input: input_v, 
			state: np.zeros((batch_size, net_size)),
			outputs_target: target_v,
		}
	)

	shl(outputs_v, target_v, file=pj("{}/{}_out.png".format(tmp_dir, e)))
	print "Epoch {}, loss {}".format(e, loss_v)
