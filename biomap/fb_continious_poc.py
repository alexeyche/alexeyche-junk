

import numpy as np
from fb_common import Sigmoid, Linear, oja_rule, norm, Learning
from util import shl, shm, shs

def smooth(signal, sigma=0.01, filter_size=50):
    lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
    return np.convolve(lf_filter, signal, mode="same")

np.random.seed(5)

act = Linear()

in_size = 3
hidden_size0 = 1
fb_size0 = 1
out_size = 1


T = 100

x_s = np.zeros((T, in_size))

x_s[25,0] = 1.0
x_s[50,1] = 1.0
x_s[75,2] = 1.0
x_s = np.asarray([smooth(x_s[:, ni]) for ni in xrange(in_size)]).T


y_t_s = np.zeros((T, out_size))
y_t_s[50,0] = 1.0
y_t_s = np.asarray([smooth(y_t_s[:, ni]) for ni in xrange(out_size)]).T



U0 = np.zeros((hidden_size0,))
U1 = np.zeros((out_size,))

F0 = np.zeros((fb_size0,))


W0 = -0.1 + 0.2*np.random.rand(in_size, hidden_size0)
W1 = -0.1 + 0.2*np.random.rand(hidden_size0, out_size)

W0_start, W1_start = W0.copy(), W1.copy()

B0 = -0.1 + 0.2*np.random.randn(out_size, fb_size0)
FB0 = -0.1 + 0.2*np.random.randn(fb_size0, hidden_size0)

stats_ep = []

lrate = 1e-01  # BP, FA
# lrate = 5e-02

lrule = Learning.HEBB
epochs = 300

for epoch in xrange(epochs):
	records = []
	stats = []
	
	fb0_s = np.zeros((T, fb_size0))

	for ti in xrange(T):
		x = x_s[ti, :]
		fb0 = fb0_s[ti, :]
		y_t = y_t_s[ti, :]

		U0 = np.dot(x, W0) + np.dot(fb0, FB0)
		a0 = act(U0)
		
		U1 = np.dot(a0, W1)
		y = act(U1)

		e = y - y_t
		error = 0.5 * np.inner(e, e)

		F0 = np.dot(e, B0)
		
		fb0_s[min(ti+1, fb0_s.shape[0]-1), :] = F0.copy()

		corr0 = np.outer(a0, x.T)
		corr1 = np.outer(y, a0)

		if lrule == Learning.BP or lrule == Learning.FA:
			if lrule == Learning.BP:
				dh0 = np.dot(W1, e) * act.deriv(a0).T
			else:
				dh0 = F0 * act.deriv(a0)

			dW0 = - np.outer(x, dh0)
			dW1 = - np.outer(a0, e)
		
		elif lrule == Learning.HEBB:
			dW0 = oja_rule(x, U0, W0, act.deriv(a0))
			dW1 = - np.outer(a0, e)

		else:
			raise NotImplementedError

		records.append(
			tuple(
				v.copy() for v in (U0, a0, U1, y, F0, dW0, dW1, corr0, corr1)
			)
		)
		
		stats.append(
			tuple(
				v.copy() for v in (e, error)
			)
		)

		

	es = np.asarray([r[0] for r in stats])
	error_s = np.asarray([r[1] for r in stats])


	U0s = np.asarray([r[0] for r in records])
	a0s = np.asarray([r[1] for r in records])
	U1s = np.asarray([r[2] for r in records])
	ys = np.asarray([r[3] for r in records])
	F0s = np.asarray([r[4] for r in records])
	dW0s = np.asarray([r[5] for r in records])
	dW1s = np.asarray([r[6] for r in records])
	corr0s = np.asarray([r[7] for r in records])
	corr1s = np.asarray([r[8] for r in records])

	W0 += lrate * np.sum(dW0s, 0)
	W1 += lrate * np.sum(dW1s, 0)

	stats_ep.append((
		corr0s, corr1s
	))
	
	if epoch % 25 == 0 or epoch == epochs-1:
		print "{}, Epoch {}, error {}".format(lrule, epoch, np.mean(error_s))

corr0ss = np.concatenate([r[0] for r in stats_ep])
corr1ss = np.concatenate([r[1] for r in stats_ep])
