

from matplotlib import pyplot as plt
import numpy as np

np.random.seed(1)

class Act(object):
	def __call__(self, x):
		raise NotImplementedError()
	
	def deriv(self, x):
		raise NotImplementedError()
		
class Linear(Act):
	def __call__(self, x):
		return x

	def deriv(self, x):
		assert len(x.shape) == 1
		return np.asarray([ 1.0 for i in xrange(len(x)) ])

class Sigmoid(Act):
	def __call__(self, x):
		return 1.0/(1.0 + np.exp(-x))

	def deriv(self, x):
		v = self(x)
		return v * (1.0 - v)

def get_oja_deriv(x, y, W, dy):
	assert W.shape[0] == len(x)
	assert W.shape[1] == len(y)

	dW = np.zeros(W.shape)
	for ni in xrange(len(y)):
		dW[:, ni] = - y[ni] * (x - y[ni] * W[:, ni]) * dy[ni]
	return dW



class Learning(object):
	BP = 0
	FA = 1
	OJA = 2

lrate = 1e-01
out_size = 10
in_size = 30
hidden_size = 20
epochs = 1000
lrule = Learning.OJA

act = Linear()

T = -1.0 + 2.0*np.random.rand(in_size, out_size)
x = np.random.randn(in_size)
y_t = act(np.dot(x, T))


W0 = -0.01 + 0.02*np.random.rand(x.shape[0], hidden_size)
W1 = -0.01 + 0.02*np.random.rand(hidden_size, out_size)

B0 = 0.01 * np.random.randn(in_size, out_size) 
B1 = 0.01 * np.random.randn(hidden_size, out_size) 

# B0_stroke = 0.01 * np.random.randn(hidden_size, out_size) 


for epoch in xrange(epochs):
	a0 = np.dot(x, W0); h0 = act(a0)
	a1 = np.dot(h0, W1); y = act(a1)

	e = y - y_t
	error = 0.5 * np.inner(e, e)
	
	xf = np.dot(B0, e)
	a0f = np.dot(xf, W0); h0f = act(a0f)
	
	# hf = np.dot(B1, e)
	# a1f = np.dot(hf, W1); h1f = act(a1f)

	# a0f = np.dot(B1, e); h0f = act(a0f)
	

	if lrule == Learning.BP or lrule == Learning.FA:
		if lrule == Learning.BP:
			dh0 = np.dot(W1, e) * act.deriv(a0)
		else:
			dh0 = np.dot(B1, e) * act.deriv(a0)
		
		dW1 = np.outer(h0, e)
		dW0 = np.outer(x, dh0)
	elif lrule == Learning.OJA:		
		dW1 = np.outer(h0, e)
		
		# x=xf; y=h0f; W=W0; dy = act.deriv(h0f)
		dW0 = get_oja_deriv(xf, h0f, W0, act.deriv(h0f))
		# dW1 = get_oja_deriv(hf, h1f, W1, act.deriv(h1f))
		
	W0 += - lrate * dW0 
	W1 += - lrate * dW1
	
	print "Epoch {}, error {}".format(epoch, error)

