from matplotlib import pyplot as plt
from matplotlib.pyplot import cm
import numpy as np
from functools import partial

from common import Determ, Learning
import scipy.sparse as sp

def get_idx_of_refractory(ti, spikes, tau_ref):
	sp_idx = spikes[max(0, ti-int(tau_ref/dt)):ti].rows
   	if len(sp_idx) > 0:
   		return [ int(v) for v in np.hstack(sp_idx) ]
   	return np.asarray([], dtype=np.uint32)
    

np.random.seed(10)

lrate = 1.0

in_size = 30
hidden_size0 = 20
out_size = 10


W0 = -1.0 + 2.0*np.random.rand(in_size, hidden_size0)
W1 = -1.0 + 2.0*np.random.rand(hidden_size0, out_size)
# W2 = -0.1 + 0.2*np.random.rand(hidden_size1, out_size)

tau_syn = 10.0
tau_ref = 2.0
tau_learn = 10.0
tau_mem = 5.0
threshold = 1.0

act = Determ(threshold)

lrule = Learning.BP

epochs = 1


F = -0.1 + 0.2*np.random.rand(in_size, out_size)

T = 100
dt = 1.0
Tsize = int(T/dt)
x_s = np.random.randn(T, in_size)
y_t_s = Determ(0.5)(np.dot(x_s, F))


u0 = np.zeros(hidden_size0)
s0 = np.zeros(hidden_size0)
u0m = np.zeros(hidden_size0)

u1 = np.zeros(out_size)
u1m = np.zeros(out_size)
ym = np.zeros(out_size)


for epoch in xrange(epochs):
    u0_stat = np.zeros((Tsize, hidden_size0))
    u0m_stat = np.zeros((Tsize, hidden_size0))
    u1_stat = np.zeros((Tsize, out_size))
    
    spikes0 = sp.lil_matrix((Tsize, hidden_size0), dtype=np.float32)
    spikes1 = sp.lil_matrix((Tsize, out_size), dtype=np.float32)
    
    int_error = 0.0
    for ti, t in enumerate(np.linspace(0, T, Tsize)):
        target_fired = y_t_s[ti,:]
        x = x_s[ti, :]
        
        u0 += dt * (- u0 + np.dot(x, W0))/tau_mem
    	u0[get_idx_of_refractory(ti, spikes0, tau_ref)] = -5.0
        a0 = act(u0)
        s0 += dt * (- s0/tau_syn + a0)
        spikes0[ti, np.where(a0 == 1.0)] = 1.0
        u0m += dt * (- u0m/tau_learn + a0)


        u1 += dt * (- u1 + np.dot(s0, W1))/tau_mem
    	u1[get_idx_of_refractory(ti, spikes1, tau_ref)] = -5.0
        a1 = act(u1)
        spikes1[ti, np.where(a1 == 1.0)] = 1.0
        u1m += dt * (- u1m/tau_learn + a1)
        
        ym += dt * (- ym/tau_learn + target_fired)

        u0_stat[ti, :] = u0
        u0m_stat[ti, :] = u0m
        u1_stat[ti, :] = u1
        
        e = ym - u1m
      
        error = 0.5 * np.inner(e, e)

        int_error += error/Tsize

    print "Epoch {}, error {}".format(epoch, int_error)  