

import numpy as np
from act import Linear, Sigmoid, Tanh, Relu
from collections import namedtuple
from util import shm, shl
import entropy_estimators as ee
import entropy as ee2
import os
from os.path import join as pj
from matplotlib import pyplot as plt

small_random_init = lambda *shape: -0.1 + 0.2*np.random.rand(*shape)

np.random.seed(1)

class LearningRule(object):
	BP = 0
	HEBB = 1


def l2_loss(y, y_t):
	return (
		np.mean(0.5*np.square(y - y_t)),
		y - y_t
	)

def cross_entropy_loss(y, y_t):
	return (
		- np.mean(y_t * np.log(y + 1e-08) + (1.0 - y_t) * np.log(1.0 - y + 1e-08)),
		y - y_t
	)

def binary_setup(x_size, y_size, batch_size):
	y = np.random.random((10*batch_size, y_size))
	y[y>0.1] = 0.0; y[y > 0.0] = 1.0
	Wy = small_random_init(y_size, x_size)
	y = y[np.where(np.sum(y, 1) != 0)]
	y = y[:batch_size, :]
	
	x = np.dot(y, Wy)
	return x, y, cross_entropy_loss


def linear_setup(x_size, y_size, batch_size):
	y = np.random.randn(batch_size, y_size)
	Wy = small_random_init(y_size, x_size)
	x = np.dot(y, Wy)
	return x, y, l2_loss



def non_linear_setup(x_size, y_size, batch_size):
	y = np.random.randn(batch_size, y_size)
	Wy = small_random_init(y_size, x_size)
	Wx = small_random_init(x_size, x_size)
	x = np.dot(np.dot(y, Wy), Wx)
	
	return x, y, l2_loss



class NeuronColumn(object):
	class Parameters(object):
		def __init__(self, W):
			self.W = W

	def __init__(self, size, act, weight_init = small_random_init):
		self._size = size
		self._act = act
		self.p = None
		self._weight_init = weight_init

	@property
	def size(self):
		return self._size
	

	def _init_p(self, x):
		return NeuronColumn.Parameters(self._weight_init(x.shape[-1], self._size))


	def __call__(self, x):
		if self.p is None:
			self.p = self._init_p(x)

		y_pre = np.dot(x, self.p.W)

		return self._act(y_pre)

	def bw(self, y):
		assert not self.p is None
		x_pre = np.dot(y, self.p.W.T)
		return self._act(x_pre)



tmp_dir = pj("{}/tmp".format(os.environ["HOME"]))
[ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f.endswith(".png")]

act = Tanh()
lrule = LearningRule.HEBB
lrate = 1e-03
epochs = 100

# if lrule == LearningRule.HEBB:
# 	lrate *= 10

y_size, x_size = 5, 25

h_size = (30, 10)



nc0 = NeuronColumn(h_size[0], act)
nc1 = NeuronColumn(h_size[1], act)
nc2 = NeuronColumn(y_size, Linear())



# x, y, loss = linear_setup(x_size, y_size, 1000)
x, y, loss = non_linear_setup(x_size, y_size, 1000)
# x, y, loss = binary_setup(x_size, y_size, 1000)


# mi = lambda a, b: np.mean([ee.mi(a, b, k = 3) for _ in xrange(1)])
mi = lambda a, b: ee.mi(a, b, k = 3)
# mi = lambda a, b: ee2.mutual_information((a, b), k=3)





info, grad_stat = [], []
for e in xrange(epochs):
	h0, afactor0 = nc0(x)
	h1, afactor1 = nc1(h0)
	y_hat, afactor2 = nc2(h1)

	error, error_deriv = loss(y_hat, y)
	
	if lrule == LearningRule.BP:
		dnc2 = error_deriv * afactor2
		dnc1 = np.dot(dnc2, nc2.p.W.T) * afactor1
		dnc0 = np.dot(dnc1, nc1.p.W.T) * afactor0
	
		dW2 = - np.dot(h1.T, dnc2)
		dW1 = - np.dot(h0.T, dnc1)
		dW0 = - np.dot(x.T, dnc0)

	elif lrule == LearningRule.HEBB:
		def hebb_rule(x, y, f, w):
			dw = np.zeros(w.shape)
			for ni in xrange(dw.shape[1]):
				for syn_i in xrange(dw.shape[0]):
					dw[syn_i, ni] = np.sum(
						f[:, ni] * (x[:, syn_i] * y[:, ni] - 10*y[:, ni] * y[:, ni] * w[syn_i, ni])
					)
			return dw

		#np.dot(nc2.p.W
		h1b, afactor2b = nc2.bw(y)
		h0b, afactor1b = nc1.bw(h1b)
		x_hat, afactor0b = nc0.bw(h0b)


		dW2b = hebb_rule(y, h1b, afactor2b, nc2.p.W.T)
		dW1b = hebb_rule(h1b, h0b, afactor1b, nc1.p.W.T)
		dW0b = hebb_rule(h0b, x_hat, afactor0b, nc0.p.W.T)

		
		# dW2 = hebb_rule(h1, y_hat, afactor2, nc2.p.W)
		# dW1 = hebb_rule(h0, h1, afactor1, nc1.p.W)
		# dW0 = hebb_rule(x, h0, afactor0, nc0.p.W)
		

		# dW2 = (dW2 + dW2b.T)/2.0
		# dW1 = (dW1 + dW1b.T)/2.0
		# dW0 = (dW0 + dW0b.T)/2.0
		dW2 = dW2b.T; dW0 = dW0b.T; dW1 = dW1b.T
		# dnc2 = y_hat
		# dnc1 = h1
		# dnc0 = h0

		# dW2 = np.dot(h1.T, dnc2) - np.dot(np.dot(y_hat.T, y_hat), nc2.p.W.T).T
		# dW1 = np.dot(h0.T, dnc1) - np.dot(np.dot(h1.T, h1), nc1.p.W.T).T
		# dW0 = np.dot(x.T, dnc0) - np.dot(np.dot(h0.T, h0), nc0.p.W.T).T


	
	nc0.p.W += lrate * dW0
	nc1.p.W += lrate * dW1
	nc2.p.W += lrate * dW2

	# y_hat = np.abs(np.ceil(y_hat-0.5))

	info.append([
		mi(h0, x), 
		mi(h1, x), 
		mi(h0, y),
		mi(h1, y), 
		#mi(np.abs(np.ceil(y_hat-0.5)), y), 
		# mi(y_hat, y),
	])
	grad_stat.append([
		np.mean(dW0), 
		np.mean(dW1),
		np.mean(dW2),
		np.var(dW0),
		np.var(dW1),
		np.var(dW2),
	])
	print "Epoch {}, error {}".format(e, error)



i = np.array(info)
shl(i[:,0], i[:,1], i[:,2], i[:,3], labels=["xh0", "xh1", "h0y", "h1y"], file=pj(tmp_dir, "info.png"))

gs = np.array(grad_stat)
shl(gs[:,0], gs[:,1], gs[:,2], labels=["m0", "m1", "m2"], file=pj(tmp_dir, "grad_mean.png"))
shl(gs[:,3], gs[:,4], gs[:,5], labels=["v0", "v1", "v2"], file=pj(tmp_dir, "grad_var.png"))
