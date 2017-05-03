
import numpy as np

net_size = 100
batch_size = 10

np.random.seed(1)

W = np.random.randn(net_size, net_size)

u = np.random.randn(batch_size, net_size)

r = np.zeros((net_size, net_size))
for i in xrange(net_size):
	for j in xrange(net_size):
		r[i, j] = W[i, j] * np.dot(u[:, i].T, u[:, j])


res = np.zeros((batch_size,))
dd = np.dot(u, W)
for bi in xrange(batch_size):
	res[bi] = np.inner(dd[bi,:], u[bi,:]) 



p = np.asarray([0, 1, 2, 3])

w = np.asarray([
	[0, 0, 1, 2], 
	[0, 0, 3, 4],
	[1, 3, 0, 0],
	[2, 4, 0, 0]
])


for i in xrange(w.shape[0]):
	for j in xrange(w.shape[1]):
		assert w[i, j] == w[j, i]

r = np.zeros((p.shape[0], p.shape[0]))
for i in xrange(p.shape[0]):
	for j in xrange(p.shape[0]):
		r[i, j] = w[i, j] * p[i] * p[j]


# import numpy as np
# import theano
# import theano.tensor as T


# n_in = 784
# n_out = 500

# pre = theano.shared(np.zeros((batch_size, net_size-10), dtype=theano.config.floatX), borrow=True)
# post = theano.shared(np.zeros((batch_size, 500), dtype=theano.config.floatX), borrow=True)


# rng = np.random.RandomState()
# W_values = np.asarray(
# 	rng.uniform(
#         low=-np.sqrt(6. / (n_in + n_out)),
#         high=np.sqrt(6. / (n_in + n_out)),
#         size=(n_in, n_out)
#     ),
#     dtype=theano.config.floatX
# )

# W = theano.shared(value=W_values, borrow=True)

# a = T.dot(pre,W)
# r = T.batched_dot(a, post)

# r_f = theano.function(
#     inputs=[],
#     outputs=[a, r, W]
# )

# a_v, r_v, W_v = r_f()

