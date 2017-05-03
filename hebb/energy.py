
import tensorflow as tf

from la_utils import batch_inner

class EnergyFunction(object):
	def __init__(self, W, bias):
		self._W = W
		self._bias = bias

	def __call__(self, V):
		raise NotImplementedError()


class HopfieldEnergy(EnergyFunction):
	def __call__(self, V):
		return - 0.5 * batch_inner(tf.matmul(V, self._W), V) - tf.matmul(V, self._bias)
			

class ExpDecayEnergy(EnergyFunction):
	def __call__(self, V):
		return (
			0.5 * tf.reduce_sum(batch_inner(V, V)) - 
			0.5 * tf.reduce_sum(batch_inner(tf.matmul(V, self._W), V)) - 
			tf.reduce_sum(tf.matmul(V, self._bias))
		)


class SparseEnergy(EnergyFunction):
	def __init__(self, W, bias, p=0.1):
		self._W = W
		self._bias = bias
		self._alpha = (1.0-p) * W.get_shape()[0].value
		self._p = p

	def sparsity_term(self, V):
		p = self._p
		return (
			p * tf.log(tf.reduce_mean(V)+1e-07) + 
			(1.0-p) * tf.log(1.0-tf.reduce_mean(V) + 1e-07)
		)


	def __call__(self, V):
		return (
			- self._alpha * self.sparsity_term(V)*V -
			0.5 * batch_inner(tf.matmul(V, self._W), V) - 
			tf.matmul(V, self._bias)
		)
		

class WtaEnergy(EnergyFunction):
	def __init__(self, W, bias, input_size):
		self._W =W 
		self._bias = bias
		self._input_size = input_size
		self._net_size = W.get_shape()[0].value


	def __call__(self, V):
		activity_sum = tf.reduce_sum(tf.slice(V, [0, self._input_size], [-1, self._net_size - self._input_size]))
		Vinh = activity_sum - V
		return (
			0.004 * tf.reduce_sum(batch_inner(Vinh, V)) - 
			0.5 * tf.reduce_sum(batch_inner(V, V)) - 
			0.5 * batch_inner(tf.matmul(V, self._W), V) - 
			tf.matmul(V, self._bias)
		)

