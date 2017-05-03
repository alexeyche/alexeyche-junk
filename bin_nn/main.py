

import numpy as np
import sys
sys.path.insert(0, "/home/alexeyche/prog/alexeyche-junk/cns/tf_dnn")
from util import shm, shl, shs

small_random_init = lambda *shape: -0.1 + 0.2*np.random.rand(*shape)
dtype = np.int8

def get_random_matrix(shape, rate=0.1):
	m = np.zeros(shape, dtype=dtype)
	m[np.random.sample(m.shape) < rate] = 1
	return m

def l2_loss(y, y_t):
	return (
		np.sum(np.square(y - y_t)),
		y - y_t
	)


class Stat(object):
	def __init__(self, seq_size, *layers_size):
		self.u = []
		for ls in layers_size:
			self.u.append(np.zeros((seq_size, ls), dtype=dtype))
		self.a = []
		for ls in layers_size:
			self.a.append(np.zeros((seq_size, ls), dtype=dtype))

		self.e = np.zeros((seq_size,), dtype=dtype)
		self.ed = np.zeros((seq_size, layers_size[-1]), dtype=dtype)

	def save(self, ti, li, uv, av):
		self.u[li][ti, :] = uv
		self.a[li][ti, :] = av

	def save_error(self, ti, ev, edv):
		self.e[ti] = ev
		self.ed[ti, :] = edv

np.random.seed(1)


net_size = 10
seq_size = 10
input_size = 10
out_size = 1
tr = 1


loss = l2_loss

xs = get_random_matrix((seq_size, input_size), 0.2)
# xs = np.zeros((seq_size, input_size))
# for i in xrange(seq_size):
# 	xs[i, i] = 1.0	


ys = get_random_matrix((seq_size, out_size), 0.2)
# ys = np.zeros((seq_size, out_size))
# ys[0,0] = 1.0
# ys[seq_size-1,0] = 1.0

W = get_random_matrix((input_size, net_size), 0.25)
Wo = get_random_matrix((net_size, out_size), 0.25)


def run_neuron(x, W):
	u = np.dot(x, W)
	fired = u >= tr
	return u, fired.astype(dtype)

for e in xrange(30):
	stat = Stat(seq_size, net_size, out_size)


	dW = np.zeros(W.shape, dtype=dtype)
	dWo = np.zeros(Wo.shape, dtype=dtype)

	for ti in xrange(seq_size):
		x = xs[ti, :]

		net_u, net_a = run_neuron(x, W)
		stat.save(ti, 0, net_u, net_a)
		
		out_u, out_a = run_neuron(net_a, Wo)
		stat.save(ti, 1, out_u, out_a)
		
		error, error_deriv = loss(out_a, ys[ti])
		stat.save_error(ti, error, error_deriv)

		dnc1 = error_deriv
		dnc0 = np.dot(dnc1, Wo.T)

		dWo += - np.outer(net_a, dnc1)
		dW += - np.outer(x, dnc0)

	# dWo[np.where(dWo > 0)] = 1
	# dWo[np.where(dWo < 0)] = -1

	# dW[np.where(dW > 0)] = 1
	# dW[np.where(dW < 0)] = -1

	W += dW.astype(dtype)
	Wo += dWo.astype(dtype)
	print "Epoch {}, error {}".format(e, np.mean(stat.e))