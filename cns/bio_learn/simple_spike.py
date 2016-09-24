
from matplotlib import pyplot as plt
from matplotlib.pyplot import cm
import numpy as np

from common import Determ, BioLearning

np.random.seed(1)

lrate = 1e-01

syn_size = 100
sim_time = 100

inp_spikes0 = [[25.0]] + [ [sim_time * np.random.random()] for _ in xrange(syn_size-1) ]
inp_spikes1 = [[]] + [ [sim_time * np.random.random()] for _ in xrange(syn_size-1) ]

T = 1
lrule = BioLearning.NEAREST_STDP


epochs = 1
dt = 1.0
y_sp_t = 75.0

W0 = 0.1*np.asarray([5.0] +  [ np.random.random() for _ in xrange(syn_size-1) ])
W1 = 0.1*np.asarray([5.0] +  [ np.random.random() for _ in xrange(syn_size-1) ])
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
tau_plus = 10
tau_minus = 10
a_plus = 1.0
a_minus = 1.0
epochs = 100

threshold = 0.5

act = Determ(threshold)

def alpha(x, tau):
    return np.exp(-x/tau)

def stdp_window(s):
    if s >= 0:
        return a_plus * alpha(s, tau_plus)
    else:
        return - a_minus * alpha(-s, tau_minus)




def srm(t, W, all_inp_spikes, out_spikes):
    x_syn = 0
    syn_fired = np.zeros(len(all_inp_spikes)) 
    for syn_id, inp_spikes in enumerate(all_inp_spikes):
        syn_fired[syn_id] = 0.0
        for t_sp in inp_spikes[:]:
            t_sp += 1.0 # dendrite time 
            if t_sp > t:
                break
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
epochs=1
for epoch in xrange(epochs):

    acc0, acc1 = [], []
    spikes0, spikes1 = [], []
    inp_spikes1[0] = spikes0

    dW0, dW1 = np.zeros(syn_size), np.zeros(syn_size)

    for t in xrange(sim_time):
        target_fired = 0.0
        if t == y_sp_t:
            target_fired = 1.0

        y0, p_act0, fired0, syn_fired0 = srm(t, W0, inp_spikes0, spikes0)
        y1, p_act1, fired1, syn_fired1 = srm(t, W1, inp_spikes1, spikes1)

        s = ""
        if fired0:
            s += "0 fired at {} ".format(t)
        if fired1:
            s += "1 fired at {}".format(t)
        if s:
            print s
        if lrule == BioLearning.NEAREST_STDP:
            for syn_id, syn_spikes in enumerate(inp_spikes1):
                if len(syn_spikes) > 0 and len(spikes1) > 0:
                    s = spikes1[-1] - syn_spikes[-1]
                    dW1[syn_id] += (target_fired - fired1)  * syn_fired1[syn_id] * stdp_window(s)
                    # print target_fired, fired0, syn_fired0[syn_id], s, stdp_window(s)

        acc0.append(y0)
        acc1.append(y1)

    
    dW0 = dW0/sim_time
    dW1 = dW1/sim_time
    
    W0 += lrate * dW0
    W1 += lrate * dW1

        # W0res[ri, ci] = W0
        # W1res[ri, ci] = W1
        # dW0res[ri, ci] = dW0
        # dW1res[ri, ci] = dW1



    if len(spikes1) == 0:
        print "Epoch {}, no spikes".format(epoch)
    else:
        print "Epoch {}, error {}".format(epoch, y_sp_t - spikes1[0])

