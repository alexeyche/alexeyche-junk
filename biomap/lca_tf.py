
from tensorflow.contrib.rnn import RNNCell as RNNCell
import tensorflow as tf

from util import *

from hist_rnn import rnn_with_hist_loop_fn
from config import Config


class LCACell(RNNCell):
    def __init__(self, layer_size, filter_size, c):
    	self._layer_size = layer_size
    	self._filter_size = filter_size
    	self._input_size = input_size
    	self._c = c
    	self._params = None

    @property
    def state_size(self):
    	return (self._layer_size, self._layer_size)

    @property
    def output_size(self):
    	return (self._layer_size, self._layer_size)

    def _init_parameters(self):
    	return tf.get_variable("F", [self._filter_size, self._input_size, self._layer_size], 
	        initializer=tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
        )

	def __call__(self, input, state, scope=None):
		####
		
		if self._params is None:
			self._params = self._init_parameters()

		x = input
		u, a = state
		F = self._params

		####
		b = tf.nn.conv1d(x, F, 1)
		Fc = tf.matmul(tf.transpose(F, (0, 2, 1), F))
		fb = tf.conv1d(a, Fc, 1)
		
		print "b", b.get_shape()
		print "Fc", Fc.get_shape()
		print "fb", fb.get_shape()

		du = - u + b - fb
		new_u = u + c.epsilon * du / c.tau

        new_a = tf.nn.relu(new_u - c.lam)
        
        ####

        return (new_u, new_a), (new_u, new_a)




input_size = 1
seq_size = 1000
batch_size = 1
layer_size = 25
filter_size = 50
L = 50

Tmax = 250
dt = 1.0
Tsize = int(Tmax/dt)
T = np.linspace(T0, Tmax, Tsize)
x_vec = 1.0*np.sin(T/10.0)


c = Config()
c.lam = 1.0
c.weight_init_factor = 1.0
c.epsilon = 1.0
c.tau = 5.0


input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")
sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)


cell = LCACell(layer_size, filter_size, c)

net_out, finstate, _ = tf.nn.raw_rnn(cell, rnn_with_hist_loop_fn(input_n, sequence_length, state, L))


sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("glm_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())

