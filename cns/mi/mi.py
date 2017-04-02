


import entropy_estimators as ee
import numpy as np
from math import floor
from util import shm, shl
import entropy as ke

def mi(x, y):
	return -ee.entropyd(np.concatenate([x, y], axis=1)) + ee.entropyd(x) + ee.entropyd(y)



def discretize(x, min_x=0.0, max_x=1.0, n_bins=10):
	x_discrete = np.zeros((x.shape[0], x.shape[1]*n_bins))
	for v_id, v in enumerate(x):
		for di, subv in enumerate(v):
			bin_id = int(floor(n_bins * (max(min(subv, max_x-1e-08), min_x)-min_x)/(max_x - min_x)))
			x_discrete[v_id, di*n_bins + bin_id] += 1
	return x_discrete


x = np.random.randn(10000, 10)
y = np.random.randn(10000, 10)

xd = discretize(x, min_x = -1.0, max_x = 1.0, n_bins=2)
yd = discretize(y, min_x = -1.0, max_x = 1.0, n_bins=2)

print mi(yd, xd)

print ee.mi(x, y, k=2)
