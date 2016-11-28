#!/usr/bin/env python


import numpy as np
from matplotlib import pyplot as plt

np.random.seed(1)

M = 100
N = 10
n_step = 15
dt = 1e-03
T = int(1.0/dt)

X = np.random.randn(T, M)


k = -0.1 + 0.2*np.random.randn(M, N)
h = -0.1 + 0.2*np.random.randn(n_step, N)
mu = -0.1 + 0.2*np.random.randn(N)


ti = 0

Y = np.zeros((T, N))
Lam = np.zeros((T, N))

for ti in xrange(T):
	y = np.zeros((n_step, N))
	step_back_ids = range(max(0, ti-n_step), max(0, ti))
	y[:len(step_back_ids), :] = Y[step_back_ids, :]

	x = X[ti,:]

	lam_in = np.dot(x, k) + np.asarray([ np.dot(h[:,ni], y[:,ni]) for ni in xrange(N) ]) + mu 

	lam = np.exp(lam_in)

	Lam[ti, :] = lam*dt
	Y[ti, :] = np.random.random(N) < lam*dt

