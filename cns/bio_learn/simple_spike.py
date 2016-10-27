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
epochs = 1

in_size = 1
hidden_size0 = 1
out_size = 1


W0 = np.random.rand(in_size, hidden_size0)
W1 = np.random.rand(hidden_size0, out_size)
B0 = np.random.rand(out_size, hidden_size0)


# W2 = -0.1 + 0.2*np.random.rand(hidden_size1, out_size)

tau_syn = 10.0
tau_ref = 5.0
tau_learn = 5.0
tau_mem = 5.0
threshold = 1.0

act = Determ(threshold)

lrule = Learning.FA


T = 40
dt = 1.0
Tsize = int(T/dt)

x_s = np.zeros((Tsize, in_size))
x_s[10,0] = 1.0

y_t_s = np.zeros((Tsize, out_size))
y_t_s[15,0] = 1.0

slice_size = 25

W0a = np.linspace(0.0, 5.0, slice_size)
W1a = np.linspace(0.0, 5.0, slice_size)


W0res = np.zeros((slice_size, slice_size))
W1res = np.zeros((slice_size, slice_size))
dW0res = np.zeros((slice_size, slice_size))
dW1res = np.zeros((slice_size, slice_size))
error_res = np.zeros((slice_size, slice_size))
spike_count0 = np.zeros((slice_size, slice_size))
spike_count1 = np.zeros((slice_size, slice_size))

for ri, W0v in enumerate(W0a):
    for ci, W1v in enumerate(W1a):
        W0 = W0v
        W1 = W1v

        for epoch in xrange(epochs):
            x_mean = np.zeros(in_size)
            u0 = np.zeros(hidden_size0)
            s0 = np.zeros(hidden_size0)
            u0m = np.zeros(hidden_size0)

            u1 = np.zeros(out_size)

            ym = np.zeros(out_size)
            ym_t = np.zeros(out_size)


            u0_stat = np.zeros((Tsize, hidden_size0))
            u0m_stat = np.zeros((Tsize, hidden_size0))
            u1_stat = np.zeros((Tsize, out_size))
            e_stat = np.zeros((Tsize, out_size))
            s0_stat = np.zeros((Tsize, hidden_size0))
            
            spikes0 = sp.lil_matrix((Tsize, hidden_size0), dtype=np.float32)
            spikes1 = sp.lil_matrix((Tsize, out_size), dtype=np.float32)

            dW0, dW1 = 0, 0

            int_error = 0.0
            for ti, t in enumerate(np.linspace(0, T, Tsize)):
                target_fired = y_t_s[ti,:]
                x = x_s[ti, :]
                
                x_mean += dt * ( - x_mean/tau_syn + x )

                u0 += dt * (- u0 + np.dot(x_mean, W0))/tau_mem
                u0[get_idx_of_refractory(ti, spikes0, tau_ref)] = -5.0
                a0 = act(u0)
                s0 += dt * (- s0/tau_syn + a0)
                spikes0[ti, np.where(a0 == 1.0)] = 1.0
                # x_mean[np.where(a0 == 1.0)] = 0.0
        
                u0m += dt * (- u0m/tau_learn + a0)

                s0_stat[ti, :] = s0

                u1 += dt * (- u1 + np.dot(s0, W1))/tau_mem
                u1[get_idx_of_refractory(ti, spikes1, tau_ref)] = -5.0
                a1 = act(u1)
                spikes1[ti, np.where(a1 == 1.0)] = 1.0
                # s0[np.where(a1 == 1.0)] = 0.0

                ym += dt * (- ym/tau_learn + a1)
                ym_t += dt * (- ym_t/tau_learn + target_fired)

                u0_stat[ti, :] = u0
                u0m_stat[ti, :] = u0m
                u1_stat[ti, :] = u1
                
                e = ym - ym_t
                e_stat[ti, :] = e

                error = 0.5 * np.inner(e, e)

                int_error += error/Tsize

                if lrule == Learning.BP or lrule == Learning.FA:
                    if lrule == Learning.BP:
                        du0 = np.dot(W1, e) #* act.deriv(u0)
                    else:
                        du0 = np.dot(e, B0) #* act.deriv(u0)
                    
                    dW0_cur = - np.outer(x_mean, du0)/Tsize
                    dW1_cur = - np.outer(s0, e)/Tsize
                    
                    dW0 += dW0_cur #- 0.0001 * W0
                    dW1 += dW1_cur #- 0.0001 * W1

            spike_count0[ri, ci] = len(np.where(spikes0.todense() > 0)[0])
            spike_count1[ri, ci] = len(np.where(spikes1.todense() > 0)[0])
            
            W0res[ri, ci] = W0
            W1res[ri, ci] = W1
            dW0res[ri, ci] = dW0
            dW1res[ri, ci] = dW1
            error_res[ri, ci] = int_error


# plt.subplot(3,1,1)
plt.figure(1)
plt.quiver(
    W0res, W1res, dW0res, dW0res,
    error_res,
    cmap=cm.seismic,     # colour map
    headlength=7, headwidth=5.0)        # length of the arrows

plt.colorbar()                      # add colour bar on the right

# plt.subplot(3,1,2)
plt.figure(2)
plt.subplot(2,1,1)
plt.imshow(spike_count0, origin='lower')
plt.colorbar()
plt.subplot(2,1,2)
plt.imshow(spike_count1, origin='lower')
plt.colorbar()                      

plt.figure(3)
plt.subplot(2,1,1)
plt.imshow(dW0res, origin='lower')
plt.colorbar()
plt.subplot(2,1,2)
plt.imshow(dW1res, origin='lower')
plt.colorbar()

plt.show()