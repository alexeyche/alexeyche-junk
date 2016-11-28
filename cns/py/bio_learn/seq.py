#!/usr/bin/env python

import numpy as np

def alpha(s):
	if s < 0:
		return 0.0
	return np.exp(-s/tau_syn)

def double_alpha(s):
	if s < 0:
		return 0.0
	return (1.0/(tau_syn1 - tau_syn2)) * (np.exp(-s/tau_syn1) - np.exp(-s/tau_syn2))


def sigmoid(x, beta):
	return 1.0/(1.0 + np.exp(-beta*x))

N = 100
M = 100
tau_syn = 10.0
tau_syn1 = 10.0
tau_syn2 = 2.0
u_0 = -5.0
beta = 0.2

T = 100
dt = 1.0
Tsize = int(T/dt)


v = np.zeros((N,))
h = np.zeros((M,))

v_target = np.zeros((Tsize, N))
v_rate = 2.0
for tt in xrange(Tsize):
    for ni in xrange(N):
        if np.random.random() <= v_rate/Tsize:        
            v_target[tt, ni] = 1.0



epsp = double_alpha

drive =  


for ti, t in enumerate(np.linspace(0, T, Tsize)):
	u_v = u_0 +  
	p_v = sigmoid(u_v, beta)

	ll = v_target[ti, :] * np.log(p_v) + (1.0 - v_target[ti, :]) * np.log(1.0 - p_v)
