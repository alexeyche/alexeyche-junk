

from matplotlib import pyplot as plt
import numpy as np


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
		dW[:, ni] = y[ni] * (x - y[ni] * W[:, ni]) * dy[ni]
	return dW

def norm(f):
	# return np.asarray([ f[ri, :] * n for ri, n in enumerate(np.sqrt(np.sum(f ** 2, 1))) ])
	return np.asarray([ f[:, ci] * n for ci, n in enumerate(np.sqrt(np.sum(f ** 2, 0))) ]).T


class Learning(object):
	BP = 0
	FA = 1
	OJA = 2
	OJA_FEED = 3

lrate = 1e-03

in_size = 50
hidden_size0 = 30
hidden_size1 = 20
out_size = 10

epochs = 1000

np.random.seed(9)


act = Sigmoid()

T = -1.0 + 2.0*np.random.rand(in_size, out_size)
x = np.random.randn(in_size)
y_t = act(np.dot(x, T))


stats = []
for lrule in (Learning.BP, Learning.FA, Learning.OJA, Learning.OJA_FEED):
	error_acc = []
	np.random.seed(1)

	W0 = -0.01 + 0.02*np.random.rand(x.shape[0], hidden_size0)
	W1 = -0.01 + 0.02*np.random.rand(hidden_size0, hidden_size1)
	W2 = -0.01 + 0.02*np.random.rand(hidden_size1, out_size)

	B0 = -0.01 + 0.02*np.random.randn(out_size, hidden_size0)
	B1 = -0.01 + 0.02*np.random.randn(out_size, hidden_size1)


	for epoch in xrange(epochs):
		a0 = np.dot(x, W0); h0 = act(a0)
		a1 = np.dot(h0, W1); h1 = act(a1)
		a2 = np.dot(h1, W2); y = act(a2)

		e = y - y_t
		error = 0.5 * np.inner(e, e)

		x0 = np.dot(e, B0); yf0 = act(x0)
		x1 = np.dot(e, B1); yf1 = act(x1)

		if lrule == Learning.BP or lrule == Learning.FA:
			if lrule == Learning.BP:
				dh0 = np.dot(W1, e) * act.deriv(a0)
				dh1 = np.dot(W2, dh0) * act.deriv(a1)
			else:
				dh0 = np.dot(e, B0) * act.deriv(a0)
				dh1 = np.dot(e, B1) * act.deriv(a0)

			dW0 = - np.outer(x, dh0)
			dW1 = - np.outer(x, dh1)
			dW2 = - np.outer(h1, e)

		elif lrule == Learning.OJA or lrule == Learning.OJA_FEED:
			dW2 = - np.outer(h1, e)

			dW0 = get_oja_deriv(x, yf0, W0, act.deriv(yf0))
			dW1 = get_oja_deriv(x, yf1, W1, act.deriv(yf1))

			# dW0 = np.zeros(dW0.shape)
			# dW1 = np.zeros(dW1.shape)

			if lrule == Learning.OJA_FEED:
				dB0 = get_oja_deriv(e, yf0, B0, act.deriv(yf0))
				dB1 = get_oja_deriv(e, yf1, B1, act.deriv(yf1))

				B0 += lrate * norm(dB0)
				B1 += lrate * norm(dB1)

		W0 += lrate * norm(dW0)
		W1 += lrate * norm(dW1)

		if epoch % 25 == 0 or epoch == epochs-1:
			print "{}, Epoch {}, error {}".format(lrule, epoch, error)

		error_acc.append(error)

	stats.append(error_acc)

plt.plot(stats[0])
plt.plot(stats[1])
plt.plot(stats[2])
plt.plot(stats[3])
plt.show()


