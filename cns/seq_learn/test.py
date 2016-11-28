
import scipy.sparse as sp
import numpy as np
from matplotlib import pyplot as plt


def rep_constant(shape, value):
    r = np.empty(shape)
    r.fill(value)
    return r

v_size = 100
h_size = 100
pop_size = v_size + h_size

# np.random.seed(25)

T = 1000
dt = 1.0
dt_sec = dt/100.0
Tsize = int(T/dt)


factor = 2.0

Wvh = - factor +  2.0 * factor*np.random.rand(v_size, h_size)
Whv = - factor +  2.0 * factor*np.random.rand(h_size, v_size)

# Wvh = rep_constant((v_size, h_size), factor)
# Whv = rep_constant((h_size, v_size), factor)


spikes = sp.lil_matrix((Tsize, pop_size), dtype=np.float32)
t_rate = 2.0
for tt in xrange(Tsize):
    for ni in xrange(v_size):
        if np.random.random() <= t_rate/Tsize:        
            spikes[tt, ni] = 1.0
# for i in xrange(10):
#     spikes[i,i] = 1.0



u_rest = -5.0
beta = 0.75
tau_decay = 10.0
tau_rise = 2.0
tau_refr = 5.0
tau_look_around = 5*tau_decay

act = lambda u: 1.0/(1.0 + np.exp(-beta * u))
epsp = lambda x: (1.0/(tau_decay - tau_rise)) * (np.exp(-x/tau_decay) - np.exp(-x/tau_rise))
post_spike_kernel = lambda x: -100.0 * np.exp(-x/tau_refr)


p_stat = np.zeros((Tsize, pop_size))
u_stat = np.zeros((Tsize, pop_size))
ll_stat = np.zeros((Tsize, pop_size))

for ti, t in enumerate(np.linspace(0, T, Tsize)):
    if ti % 100 == 0:
        print "T: {}".format(t)

    past_idx = max(0, int(ti-dt*tau_look_around))

    spikes_past = spikes[past_idx:ti, :].todense()

    u = rep_constant((pop_size,), u_rest)

    spike_times, neuron_ids = np.where(spikes_past)
    for t_sp, n_id in zip(spike_times, neuron_ids):
        t_sp += t - spikes_past.shape[0]*dt
        if n_id < v_size:
            u[v_size:] += Wvh[n_id, :] * epsp(t - t_sp)
        else:
            u[:v_size] += Whv[n_id-v_size, :] * epsp(t - t_sp)
            
        u[n_id] += post_spike_kernel(t - t_sp)

    p = act(u)

    curr_spikes = p > np.random.rand(p.shape[0])
    
    ll = 0
    
    for n_id in np.where(curr_spikes == 1.0)[0]:
        if n_id < v_size:
            continue
        spikes[ti, n_id] = 1.0

    u_stat[ti, :] = u
    p_stat[ti, :] = p
    x = spikes[ti, :].todense().T
    ll_stat[ti, :] = x * np.log(p) + (1.0 - x) * np.log(1.0 - p)


plt.figure(1)
plt.plot(p_stat[:,v_size + 0])
plt.figure(2)
plt.imshow(spikes.todense().T, cmap='gray')
plt.show()

