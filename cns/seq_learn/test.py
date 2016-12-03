
import scipy.sparse as sp
import numpy as np
from matplotlib import pyplot as plt


def rep_constant(shape, value):
    r = np.empty(shape)
    r.fill(value)
    return r


class Stats(object):
    def __init__(self, epochs, Tsize, pop_size):
        self.p = np.zeros((Tsize, pop_size))
        self.u = np.zeros((Tsize, pop_size))
        self.ll = np.zeros((Tsize, pop_size))
        self.W = np.zeros((Tsize, pop_size, pop_size))
        self.epsp = np.zeros((Tsize, pop_size))


# np.random.seed(25)

v_size = 100
h_size = 0
pop_size = v_size + h_size
u_rest = -5.0
beta = 1.0
tau_decay = 10.0
tau_rise = 2.0
tau_refr = 5.0
tau_look_around = 5*tau_decay
lrate = 100.0
T = 100
dt = 1.0
dt_sec = dt/1000.0
Tsize = int(T/dt)
weight_factor = 0.0
W = - weight_factor +  2.0 * weight_factor*np.random.rand(pop_size, pop_size)
epochs = 50

act = lambda u: 1.0/(1.0 + np.exp(-beta * u))
epsp = lambda x: (1.0/(tau_decay - tau_rise)) * (np.exp(-x/tau_decay) - np.exp(-x/tau_rise))
post_spike_kernel = lambda x: -100.0 * np.exp(-x/tau_refr)
act_factor = lambda _: beta

def step(ti, t, spikes, stats, simulate_visible=False, learn=True):
    past_idx = max(0, int(ti-dt*tau_look_around))

    spikes_past = spikes[past_idx:ti, :].todense()

    u = rep_constant((pop_size,), u_rest)

    spike_times, neuron_ids = np.where(spikes_past)

    for t_sp, n_id in zip(spike_times, neuron_ids):
        t_sp += t - spikes_past.shape[0]*dt
        x_e = epsp(t - t_sp)
        u += W[n_id, :] * x_e

        stats.epsp[ti, n_id] = x_e

        u[n_id] += post_spike_kernel(t - t_sp)

    p = act(u)

    curr_spikes = p > np.random.rand(p.shape[0])

    for n_id in np.where(curr_spikes == 1.0)[0]:
        if not simulate_visible and n_id < v_size:
            continue
        spikes[ti, n_id] = 1.0

    x = np.squeeze(np.array(spikes[ti, :].todense()))

    if learn:
        for t_sp, n_id in zip(spike_times, neuron_ids):
            t_sp += t - spikes_past.shape[0]*dt
            W[n_id, :] += lrate * act_factor(u) * (x - p) * epsp(t - t_sp)

    stats.u[ti, :] = u
    stats.p[ti, :] = p
    stats.ll[ti, :] = x * np.log(p) + (1.0 - x) * np.log(1.0 - p)





spikes = sp.lil_matrix((Tsize, pop_size), dtype=np.float32)

# t_rate = 2.0
# for tt in xrange(Tsize):
#     for ni in xrange(v_size):
#         if np.random.random() <= t_rate/Tsize:
#             spikes[tt, ni] = 1.0


for vi in xrange(v_size):
    spikes[vi, vi] = 1.0



stats = Stats(epochs, Tsize, pop_size)

for ep in xrange(epochs):
    spikes_run = spikes.copy()
    for ti, t in enumerate(np.linspace(0, T, Tsize)):
        step(ti, t, spikes_run, stats, simulate_visible=False, learn=True)


    print "Epoch {} finished, likelihood: {}".format(ep, np.mean(stats.ll))
    stats.W[ep, :, :] = W



spikes_eval = sp.lil_matrix((Tsize, pop_size), dtype=np.float32)
for ti, t in enumerate(np.linspace(0, T, Tsize)):
    step(ti, t, spikes_eval, stats, simulate_visible=True, learn=False)


plt.figure(1)
plt.imshow(spikes_eval.todense().T, cmap='gray')
plt.figure(2)
plt.imshow(spikes.todense().T, cmap='gray')
plt.show()

