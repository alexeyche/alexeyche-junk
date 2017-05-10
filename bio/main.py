import gc
import numpy as np
from util import shl, shm, shs
from functools import partial
from activation import *
from datasets import get_toy_data, quantize_data
import time

def step(x):
    return 1 * (x >= 0)

def epsp_kernel(t, tau_s, tau_l):
    koef = 1.0/(tau_l - tau_s)
    return step(t) * (np.exp(-t/tau_l) - np.exp(-t/tau_s)) * koef

def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)

def smooth_spikes(t, sp, dim_size, kernel):
    sp_smooth = np.zeros(dim_size)
    for t_sp, n_id  in sp:
        sp_smooth[n_id] += kernel(t-t_sp)
    return sp_smooth

def smooth_spikes_batch(t, sp, batch_size, dim_size, kernel):
    sp_smooth = np.zeros((batch_size, dim_size))
    for t_sp, b_id, n_id   in sp:
        sp_smooth[b_id, n_id] += kernel(t-t_sp)
    return sp_smooth

def one_hot(y, y_size):
    y_oh = np.zeros((y.shape[0], y_size))
    y_oh[np.arange(y.shape[0]), y] = 1.0
    return y_oh


def smooth_input_and_output(x_values, y_values, Tsize, lambda_max, koeff_epsp, tau_s, tau_l):
    n_samples = x_values.shape[0]
    input_size = x_values.shape[1]
    output_size = y_values.shape[1]

    x_values_smooth = np.zeros((Tsize, n_samples, input_size), dtype=np.float32)
    y_values_smooth = np.zeros((Tsize, n_samples, output_size), dtype=np.float32)

    x_values_smooth_state = np.zeros((n_samples, input_size), dtype=np.float32)
    y_values_smooth_state = np.zeros((n_samples, output_size), dtype=np.float32)

    x_values_smooth_aux = np.zeros((n_samples, input_size), dtype=np.float32)
    y_values_smooth_aux = np.zeros((n_samples, output_size), dtype=np.float32)

    for ti, t in enumerate(np.linspace(T0, T, Tsize)):
        x_realisation = poisson(lambda_max * x_values)
        y_realisation = poisson(lambda_max * y_values)

        x_values_smooth_aux += koeff_epsp * x_realisation
        x_values_smooth_state += 1.5 * koeff_epsp * x_values_smooth_aux

        y_values_smooth_aux += koeff_epsp * y_realisation
        y_values_smooth_state += 1.5 * koeff_epsp * y_values_smooth_aux

        x_values_smooth[ti, :, :] = x_values_smooth_state
        y_values_smooth[ti, :, :] = y_values_smooth_state

        x_values_smooth_state -= dt * x_values_smooth_state/tau_l
        x_values_smooth_aux -= dt * x_values_smooth_aux/tau_s

        y_values_smooth_state -= dt * y_values_smooth_state/tau_l
        y_values_smooth_aux -= dt * y_values_smooth_aux/tau_s

    del x_values_smooth_aux, x_values_smooth_state
    del y_values_smooth_aux, y_values_smooth_state

    gc.collect()
    return x_values_smooth, y_values_smooth

tau_s = 3.0  # tau rise
tau_l = 10.0 # tau decay

epsp_kernel = partial(epsp_kernel, 
    tau_s=tau_s, 
    tau_l=tau_l
)

koeff_epsp = 1.0/(tau_l - tau_s)

smooth_spikes = partial(smooth_spikes, kernel=epsp_kernel)
smooth_spikes_batch = partial(smooth_spikes_batch, kernel=epsp_kernel)

gL = 0.1
gD = 0.6
gB = 0.6
gA_0, gA_2, gA_2 = 0.0, 0.05, 0.6

gA = 0.0

T0, T = 0.0, 100.0      # ms
dt = 1.0                # ms

Ee = 8.0
Ei = -8.0

poisson = partial(poisson, dt=dt)

Tsize = int(T/dt + dt)
lambda_max = 200.0/1000.0      # 1/ms

act = SigmoidActivation()

x_values, y_values = get_toy_data(seed=2)
n_train = (4 * x_values.shape[0]/5)
n_valid = (1 * x_values.shape[0]/5)


y_uniq = np.unique(y_values)
y_hot = one_hot(y_values, len(y_uniq))


input_size = x_values.shape[1]
input_len = x_values.shape[0]
hidden_size = 100
output_size = len(y_uniq)


batch_size = 1
batch_to_listen = 0

L, M, N = input_size, hidden_size, output_size

#######################################################################

A = np.zeros((batch_size, hidden_size))         # feedback
B = np.zeros((batch_size, hidden_size))         # feedforward
C = np.zeros((batch_size, hidden_size))         # soma
U = np.zeros((batch_size, output_size))         

W0 = 0.1*(np.random.uniform(size=(input_size, hidden_size)) - 0.5)
b0 = np.zeros((hidden_size,))
W1 = 0.1*(np.random.uniform(size=(hidden_size, output_size)) - 0.5)
b1 = np.zeros((output_size,))
Y = np.random.uniform(size=(output_size, hidden_size)) - 0.5

A_stat = np.zeros((Tsize, hidden_size))
B_stat = np.zeros((Tsize, hidden_size))
C_stat = np.zeros((Tsize, hidden_size))
X_stat = np.zeros((Tsize, input_size))
C_rates_stat = np.zeros((Tsize, hidden_size))
U_rates_stat = np.zeros((Tsize, output_size))

act_h = np.zeros((batch_size, hidden_size,))
act_aux_h = np.zeros((batch_size, hidden_size,))

act_o = np.zeros((batch_size, output_size,))
act_aux_o = np.zeros((batch_size, output_size,))

x_values_sm, y_values_sm = smooth_input_and_output(x_values, y_hot, Tsize, lambda_max, koeff_epsp, tau_s, tau_l)

n_train = 100

time_acc = 0.0
for xi in xrange(n_train):
    hidden_spikes = []
    for bi in xrange(batch_size):
        for ni in xrange(output_size):
            act_o[bi, ni] = 0.0
            act_aux_o[bi, ni] = 0.0
        
        for ni in xrange(hidden_size):
            act_h[bi, ni] = 0.0
            act_aux_h[bi, ni] = 0.0

    y = y_hot[xi]
    gE = y
    gI = 1.0 - y

    start = time.time()
    for ti, t in enumerate(np.linspace(T0, T, Tsize)):
        x = x_values_sm[ti, xi, :]
        
        B = np.dot(x, W0) + b0
        A = np.dot(act_o, Y) 

        C += dt * (- gL * C + gB * (B - C) + gA * (A - C))

        lambda_C = act(C)
        lambda_C_r = poisson(lambda_max * lambda_C)
        
        act_aux_h += koeff_epsp * lambda_C_r
        act_h += 1.5 * koeff_epsp * act_aux_h

        V = np.dot(act_h, W1) + b1

        I = gE * (Ee - U) + gI * (Ei - U)

        U += dt * (- gL * U + gD * (V - U) + I)

        lambda_U = act(U)
        lambda_U_r = poisson(lambda_max * lambda_U)
        
        act_aux_o += koeff_epsp * lambda_U_r
        act_o += 1.5 * koeff_epsp * act_aux_o
        
        C_stat[ti] = C[batch_to_listen]
        C_rates_stat[ti] = act_h[batch_to_listen]
        U_rates_stat[ti] = act_o[batch_to_listen]

        act_o -= dt * act_o/tau_l
        act_aux_o -= dt * act_aux_o/tau_s
        act_h -= dt * act_h/tau_l
        act_aux_h -= dt * act_aux_h/tau_s

    print xi, time.time() - start
    time_acc += time.time() - start

print time_acc/n_train