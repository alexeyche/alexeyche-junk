

import numpy as np
from util import shl

def step(x):
    return 1 * (x >= 0)




tau_s = 3.0  # tau rise
tau_l = 10.0 # tau decay

k0 = tau_s*tau_l/(tau_l - tau_s)
k1 = 1.0/(tau_l - tau_s)

def epsp_kernel(t, tau_s, tau_l):
    return step(t) * (np.exp(-t/tau_l) - np.exp(-t/tau_s)) * k1


g = 0
h = 0
i = 0
h0 = 1.0
dt = 1.0

t = np.linspace(0, 100, 100)

stat = np.zeros((2, 100,))
for ti, _ in enumerate(t):
    if ti == 0:
        i = 1.0
    else:
        i = 0.0

    g +=  - dt * g/tau_l + 1.5 * k1 * h
    h +=  - dt * h/tau_s + k1 * i
    
    stat[0, ti] = g
    stat[1, ti] = h



ans = epsp_kernel(t, tau_s, tau_l)


shl(ans, stat[0,:])