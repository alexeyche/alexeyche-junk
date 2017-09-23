
import tensorflow as tf
from math import ceil

class AutoEncoder(object):
	def __init__(self, name, shape, act, tied_weights=False, weight_init_factor=1.0):
		self._tied_weights = tied_weights
		self._act = act
		self._name = name
		self._weight_init_factor = weight_init_factor
		self._shape = shape
		if self._tied_weights:
			assert len(self._shape) % 2 == 1, "Need odd number of layers for tied weights"
			middle = int(ceil(len(self._shape)/2.0))
			assert self._shape[:(middle-1)] == list(reversed(self._shape[middle:])), "Need symmetric layer shape for tied weights: {} != {}".format(self._shape[:(middle-1)],list(reversed(self._shape[middle:])))
			self._shape = self._shape[:middle]
				
		self._vars = []
		self._additional_bias = []

	def _init_parameters(self, x):
		input_size = x.get_shape()[1].value

		for ni, layer_size in enumerate(self._shape):
			prev_layer_size = input_size if ni == 0 else self._shape[ni-1]

			self._vars.append((
				tf.get_variable("{}-W{}".format(self._name, ni), (prev_layer_size, layer_size), 
			    	initializer=tf.uniform_unit_scaling_initializer(factor=self._weight_init_factor)
				),
				tf.get_variable("{}-b{}".format(self._name, ni), (layer_size,), 
			    	initializer=tf.zeros_initializer()
				),
			))

		if not self._tied_weights:
			ni = len(self._shape)
			prev_layer_size = self._shape[-1]
			layer_size = input_size

			self._vars.append((
				tf.get_variable("{}-W{}".format(self._name, ni), (prev_layer_size, layer_size), 
			    	initializer=tf.uniform_unit_scaling_initializer(factor=self._weight_init_factor)
				),
				tf.get_variable("{}-b{}".format(self._name, ni), (layer_size,), 
			    	initializer=tf.zeros_initializer()
				),
			))
		else:
			for ni, layer_size in reversed(list(enumerate(self._shape[:-1]))):
				prev_layer_size = input_size if ni == 0 else self._shape[ni-1]

				self._additional_bias.append(
					tf.get_variable("{}-rec-b{}".format(self._name, ni), (layer_size,), 
			    		initializer=tf.zeros_initializer()
					)
				)


	def __call__(self, x):
		if len(self._vars) == 0:
			self._init_parameters(x)
		
		net_outputs = []
		for ni, (W, b) in enumerate(self._vars):
			prev_output = x if ni == 0 else net_outputs[ni-1]
			
			if not self._tied_weights and ni == len(self._vars)-1:
				net_outputs.append(
					tf.matmul(prev_output, W) + b
				)
				continue
			
			net_outputs.append(
				self._act(tf.matmul(prev_output, W) + b)
			)

		if self._tied_weights:
			bw_net_outputs = []
			for ni, (W, b) in enumerate(reversed(self._vars)):
				prev_output = net_outputs[-1] if ni == 0 else bw_net_outputs[ni-1]
				
				bw_net_outputs.append(
					tf.matmul(prev_output, tf.transpose(W))
				)
			net_outputs += bw_net_outputs
			l = tf.nn.l2_loss(bw_net_outputs[-1] - x)
		else:
			l = tf.nn.l2_loss(net_outputs[-1] - x)

		flat_vars = [v for t in self._vars for v in t]
		dvars = tf.gradients(l, flat_vars)

		return net_outputs, l, list(zip(dvars, flat_vars))


