
from matplotlib import pyplot as plt
from matplotlib.pyplot import cm
import numpy as np
from functools import partial

from common import Determ, BioLearning

np.random.seed(10)

lrate = 1.0

syn_size = 100
sim_time = 100

inp_spikes0 = [ [sim_time * np.random.random()] for _ in xrange(syn_size-1) ]
inp_spikes1 = [ [sim_time * np.random.random()] for _ in xrange(syn_size-1) ]

T = 1
lrule = BioLearning.SPAN


dt = 1.0
target_spikes = [75.0]

W0 = np.asarray([ np.random.random() for _ in xrange(syn_size) ])
W1 = np.asarray([ np.random.random() for _ in xrange(syn_size) ])
B0 = 1.0

slice_size = 25

W0a = np.linspace(0, 1.0, slice_size)
W1a = np.linspace(0, 1.0, slice_size)

W0res = np.zeros((slice_size, slice_size))
W1res = np.zeros((slice_size, slice_size))
dW0res = np.zeros((slice_size, slice_size))
dW1res = np.zeros((slice_size, slice_size))
error_res = np.zeros((slice_size, slice_size))


tau_syn = 10.0
tau_ref = 2.0
tau_plus = 5.0
tau_minus = 10.0
a_plus = 1.0
a_minus = 1.0
epochs = 100

threshold = 0.5

act = Determ(threshold)

def alpha(x, tau):
    if x < 0:
        return 0.0
    return np.exp(-float(x)/tau)

def stdp_window(s):
    if s >= 0:
        return a_plus * alpha(s, tau_plus)
    else:
        return - a_minus * alpha(-s, tau_minus)



def conv(t, spikes, kernel):
    v = 0.0
    for t_sp in spikes:
        if t_sp > t:
            break
        v += kernel(t-t_sp)
    return v

def srm(t, out_spikes, W=None, all_inp_spikes=[], x_syn=None):
    syn_fired = None
    if x_syn is None:
        x_syn = 0
        syn_fired = np.zeros(len(all_inp_spikes))
        for syn_id, inp_spikes in enumerate(all_inp_spikes):
            syn_fired[syn_id] = 0.0
            for t_sp in inp_spikes[:]:
                if t_sp > t:
                    break
                t_sp += 1.0 # dendrite time
                s_sp = t - t_sp
                if abs(s_sp) < dt:
                    syn_fired[syn_id] = 1.0
                if s_sp < tau_syn*10:
                    x_syn += W[syn_id] * alpha(s_sp, tau_syn)
                else:
                    inp_spikes.pop(0)

    y = x_syn
    if len(out_spikes) > 0:
        y += -100.0 * alpha(t-out_spikes[-1], tau_ref)
    p_act = act(y)
    fired = 0.0
    if p_act > np.random.random():
        out_spikes.append(t)
        fired = 1.0
    return y, p_act, fired, syn_fired

ri=0
ci=0
epochs=1000
for epoch in xrange(epochs):
    target_id = 0
    acc0, acc1 = [], []
    spikes0, spikes1, spikes_e_plus, spikes_e_minus = [], [], [], []
    inp_spikes1[0] = spikes0

    dW0, dW1 = np.zeros(syn_size), np.zeros(syn_size)

    cum_error = 0
    for t in xrange(sim_time):
        target_fired = 0.0
        if target_id < len(target_spikes) and t == target_spikes[target_id]:
            target_id += 1
            target_fired = 1.0

        y0, p_act0, fired0, syn_fired0 = srm(t, spikes0, W0, inp_spikes0)
        y1, p_act1, fired1, syn_fired1 = srm(t, spikes1, W1, inp_spikes1)

        mean_y = conv(t, spikes1, partial(alpha, tau=tau_plus))
        mean_y_t = conv(t, target_spikes, partial(alpha, tau=tau_plus))
        e = mean_y_t - mean_y

        y_e_plus, _, fired_e_plus, syn_fired_e_plus = srm(t, spikes_e_plus, x_syn=e)

        
        cum_error += e 

        if lrule == BioLearning.RESUME:
            for syn_id, syn_spikes in enumerate(inp_spikes1):
                dW1[syn_id] += (target_fired - fired1)  * conv(t, syn_spikes, partial(alpha, tau=tau_plus))
        elif lrule == BioLearning.SPAN:    
            for syn_id, syn_spikes in enumerate(inp_spikes1):
                mean_x = conv(t, syn_spikes, partial(alpha, tau=tau_plus))
                dW1[syn_id] += (mean_y_t - mean_y)  * mean_x

            # B0

        acc0.append(y0)
        acc1.append(y1)

    cum_error = cum_error/sim_time

    dW0 = dW0/sim_time
    dW1 = dW1/sim_time

    W0 += lrate * dW0
    W1 += lrate * dW1

    # W0res[ri, ci] = W0
    # W1res[ri, ci] = W1
    # dW0res[ri, ci] = dW0
    # dW1res[ri, ci] = dW1


    if len(spikes1) == 0:
        print "Epoch {}, no spikes, cum_error {}".format(epoch, cum_error)
    else:
        print "Epoch {}, error {}, cum_error {}".format(epoch, target_spikes[0] - spikes1[0], cum_error)

