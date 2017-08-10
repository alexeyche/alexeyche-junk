
import time

from util import shl, shm, shs
from sklearn.datasets import make_classification
from matplotlib import pyplot as plt

import numpy as np


sigmoid = lambda x: 1.0/(1.0 + np.exp(-x))
linear = lambda x: x

def relu(x):
    x[x<0] = 0.0 
    return x

plx = lambda x, y: shs(x[np.where(y == 0)], x[np.where(y == 1)], labels=["#FF0000", "#0000FF"])


def norm(w):
	return np.asarray([w[:,i]/np.sqrt(np.sum(np.square(w[:,i]))) for i in xrange(w.shape[1])]).T

def bcm_rule(x, y, w, ym):
	return np.dot(x.T, y * (y - ym))/x.shape[0]

def hebb_rule(x, y, w, ym):
	return np.dot(x.T, y)/x.shape[0]

def oja_rule(x, y, w, ym):
	assert w.shape[1] == y.shape[1]
	dw = np.zeros(w.shape)
	for ni in xrange(y.shape[1]):
		y_n = y[:, ni]
		x_v_norm = x - (y_n * w[:, ni].reshape(w.shape[0], 1)).T

		dw[:, ni] = np.mean(y_n.reshape((y_n.shape[0], 1)) * x_v_norm , 0)
	
	return dw

##

seed = 3
n_samples = 2000
n_features = 2
net_size = 10
act = linear
epochs = 200
lrate = 0.05
ym_rate = 0.9
threshold = None
batch_size = 200
tau = 5.0
dt = 1.0

##


np.random.seed(seed)

x_v, y_v = make_classification(        
    n_samples=n_samples,
    n_features=n_features, 
    random_state=seed,
    n_redundant=0,
    n_repeated=0,
)




w0 = np.random.randn(n_features, net_size)
w0 = norm(w0)

v0 = np.random.randn(net_size, net_size)
v0 = norm(v0)
np.fill_diagonal(v0, 0)

wd = []

for rule in [hebb_rule]: #, oja_rule, bcm_rule]:
# for (threshold, rule) in [(-1.0, bcm_rule), (0.0, bcm_rule), (0.5, bcm_rule), (1.0, bcm_rule), (None, bcm_rule)]:	
	w = w0.copy()
	v = v0.copy()

	ww = [w.copy()]
	vv = [v.copy()]


	a0m = np.zeros((net_size,))
	a0mm = []

	u0 = np.zeros((batch_size, net_size,))
	a0 = np.zeros((batch_size, net_size,))

	for e in xrange(epochs):
		for bi0 in xrange(0, x_v.shape[0], batch_size):
			x = x_v[bi0:(bi0+batch_size)]
			
			u0 += dt * (-u0 + np.dot(x, w) - np.dot(a0, v)) / tau
					
			a0 = act(u0)
			print np.linalg.norm(np.dot(a0.T, a0)/a0.shape[0], 2)

			if e == 0:
				a0m = np.mean(a0, 0)
			a0m = ym_rate * a0m + (1.0 - ym_rate) * np.mean(a0, 0)

			dw = rule(x, a0, w, a0m if threshold is None else threshold)
			
			w += lrate * dw
			w = norm(w)
			
			
			dv = rule(a0, a0, v, a0m if threshold is None else threshold)
			if np.linalg.norm(dv, 2) > 100:
				raise Exception()

			v += lrate * (- dv)
			v = norm(v)
			np.fill_diagonal(v, 0.0)

			ww.append(w.copy())
			vv.append(v.copy())
			a0mm.append(a0m.copy())
	
		if e % 25 == 0:
			print "Epoch {}, {}".format(e, rule.__name__)

	ww = np.asarray(ww)
	a0mm = np.asarray(a0mm)

	vv = np.asarray(vv)

	wd.append(("{}{}".format(rule.__name__, "_{}".format(threshold) if not threshold is None else ""), ww.copy()))


for wname, ww in wd:
	shl(*[ww[:,i,:] for i in xrange(ww.shape[1])], title=wname, show=False)
plt.show()

ev, em = np.linalg.eig(np.cov(x_v.T))
em = em[:, list(reversed(np.argsort(ev)))]

