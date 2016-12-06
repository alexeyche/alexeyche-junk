
import scipy.sparse as sp
import numpy as np
from matplotlib import pyplot as plt


def rep_constant(shape, value):
    r = np.empty(shape)
    r.fill(value)
    return r

def safe_ln(x, minval=0.0000000001):
    # return np.log(x)
    return np.log(x.clip(min=minval))

class Stats(object):
    def __init__(self, epochs, Tsize, pop_size):
        self.p = np.zeros((Tsize, pop_size))
        self.u = np.zeros((Tsize, pop_size))
        self.ll = np.zeros((Tsize, pop_size))
        self.epsp = np.zeros((Tsize, pop_size))
        self.delta_r = np.zeros((Tsize,))
        self.r = np.zeros((Tsize,))
        self.mean_r = np.zeros((Tsize,))
        
class State(object):
    def __init__(self, pop_size):
        self.r = 0.0
        self.mean_r = 0.0
        self.spikes_smoothed = np.zeros((pop_size,))
        self.trace = np.zeros((pop_size,pop_size))
        self.dW = np.zeros((pop_size,pop_size))

np.random.seed(25)

v_size = 25
h_size = 25
pop_size = v_size + h_size
u_rest = -5.0
beta = 0.75
tau_decay = 10.0
tau_rise = 2.0
tau_refr = 5.0
tau_look_around = 20*tau_decay


# tau_trace = 10.0
# tau_mean_trace = 100.0


T = 50.0
dt = 1.0
dt_sec = dt/1000.0
Tsize = int(T/dt)
weight_factor = 0.0
W = - weight_factor +  2.0 * weight_factor*np.random.rand(pop_size, pop_size)

lambda_first = 1.0/T
lambda_second = lambda_first/10.0

lrate = 1.0
epochs = 10000

act = lambda u: 1.0/(1.0 + np.exp(-beta * u))
epsp = lambda x: (1.0/(tau_decay - tau_rise)) * (np.exp(-x/tau_decay) - np.exp(-x/tau_rise))
post_spike_kernel = lambda x: -100.0 * np.exp(-x/tau_refr)
act_factor = lambda _: beta

def step(ti, t, spikes, state, stats, simulate_visible=False, learn=True):
    spikes_smoothed = np.zeros((pop_size,))

    past_idx = max(0, int(ti-dt*tau_look_around))

    spikes_past = spikes[past_idx:ti, :].todense()

    u = rep_constant((pop_size,), u_rest)

    spike_times, neuron_ids = np.where(spikes_past)

    for t_sp, n_id in zip(spike_times, neuron_ids):
        t_sp += t - spikes_past.shape[0]*dt
        
        x_j = epsp(t - t_sp)

        spikes_smoothed[n_id] += x_j
        u += W[n_id, :] * x_j
        
        stats.epsp[ti, n_id] = spikes_smoothed[n_id]
        
        u[n_id] += post_spike_kernel(t - t_sp)

    p = act(u)

    curr_spikes = p > np.random.rand(p.shape[0])

    for n_id in np.where(curr_spikes == 1.0)[0]:
        if not simulate_visible and n_id < v_size:
            continue
        spikes[ti, n_id] = 1.0

    x = np.squeeze(np.array(spikes[ti, :].todense()))

    ll = x * safe_ln(p) + (1.0 - x) * safe_ln(1.0 - p)

    if np.any(np.isnan(ll)):
        raise Exception("NAN: \nll {}\np: {}, W: {}".format(ll, p, W))

    state.mean_r = (1.0 - lambda_second) * state.mean_r + lambda_second * state.r   # need state.r(t-1)
    state.r = (1.0 - lambda_first) * state.r + lambda_first * np.sum(ll[:v_size])

    if learn:
        factor = state.r - state.mean_r    
        for _, n_id in zip(spike_times, neuron_ids):
            # state.trace[n_id, :] = (1.0 - lambda_first) * state.trace[n_id, :] + lambda_first * (
            #     act_factor(u) * (x - p) * state.spikes_smoothed[n_id]
            # )
            # W[n_id, :] += lrate * state.trace[n_id, :] * (1.0 if n_id < v_size else factor)
            # W[n_id, :] = W[n_id, :].clip(-100.0, 100.0)
        
            for i in xrange(pop_size):
                state.dW[n_id, i] += lrate * act_factor(u[i]) * (x[i] - p[i]) * spikes_smoothed[n_id] * (1.0 if i < v_size else factor)
            
            if np.any(np.isinf(W[n_id, :])):
                raise Exception("INF: p: {}, factor: {}, act_factor: {}, x: {}, epsp: {}".format(
                    p, factor, act_factor(u), x, spikes_smoothed[n_id])
                )


    # state.mean_r += dt * (state.r - state.mean_r)/tau_mean_trace
    # state.r += dt * (ll - state.r)/tau_trace

    stats.u[ti, :] = u
    stats.p[ti, :] = p
    stats.ll[ti, :] = ll
    stats.delta_r[ti] = state.r - state.mean_r
    stats.r[ti] = state.r
    stats.mean_r[ti] = state.mean_r
    
    



given_spikes = sp.lil_matrix((Tsize, pop_size), dtype=np.float32)

# t_rate = 1.0
# for tt in xrange(Tsize):
#     for ni in xrange(v_size):
#         if np.random.random() <= t_rate/Tsize:
#             given_spikes[tt, ni] = 1.0


for vi in xrange(v_size):
    given_spikes[vi, vi] = 1.0
    given_spikes[Tsize - vi-1, vi] = 1.0



stats = Stats(epochs, Tsize, pop_size)

r, mean_r = 0.0, 0.0

for ep in xrange(epochs):
    state = State(pop_size)
    state.r, state.mean_r = r, mean_r
    
    spikes = given_spikes.copy()
    for ti, t in enumerate(np.linspace(0, T, Tsize)):
        step(ti, t, spikes, state, stats, simulate_visible=False, learn=True)

    W += state.dW
    W = W.clip(-100.0, 100.0)
    r, mean_r = state.r, state.mean_r
    print "Epoch {} finished, likelihood: {}".format(ep, np.mean(stats.ll))



stats_eval = Stats(epochs, Tsize, pop_size)
state_eval = State(pop_size)

spikes_eval = sp.lil_matrix((Tsize, pop_size), dtype=np.float32)
spikes_eval[:3,:] = given_spikes[:3,:]
for ti, t in enumerate(np.linspace(0, T, Tsize)):
    step(ti, t, spikes_eval, state_eval, stats_eval, simulate_visible=True, learn=False)


plt.figure(1)
plt.imshow(given_spikes.todense().T, cmap='gray')
plt.figure(2)
plt.imshow(spikes_eval.todense().T, cmap='gray')
plt.figure(3)
plt.imshow(W) 
plt.colorbar()
plt.show()

