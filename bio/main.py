
import numpy as np
from util import shl, shm, shs
from functools import partial
from activation import *
from datasets import get_toy_data, quantize_data

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



epsp_kernel = partial(epsp_kernel, 
    tau_s=3.0, 
    tau_l=10.0
)

smooth_spikes = partial(smooth_spikes, kernel=epsp_kernel)
smooth_spikes_batch = partial(smooth_spikes_batch, kernel=epsp_kernel)

gL = 0.1
gD = 0.6
gB = 0.6
gA_0, gA_2, gA_2 = 0.0, 0.05, 0.6

gA = 0.0

T0, T = 0.0, 100.0      # ms
dt = 1.0                # ms

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
hidden_size = 100
output_size = len(y_uniq)

batch_size = 1
batch_to_listen = 0

L, M, N = input_size, hidden_size, output_size

#######################################################################

A = np.zeros((batch_size, hidden_size))         # feedback
B = np.zeros((batch_size, hidden_size))         # feedforward
C = np.zeros((batch_size, hidden_size))         # soma

W0 = 0.1*(np.random.uniform(size=(input_size, hidden_size)) - 0.5)
W1 = 0.1*(np.random.uniform(size=(hidden_size, output_size)) - 0.5)
Y = np.random.uniform(size=(output_size, hidden_size)) - 0.5

A_stat = np.zeros((Tsize, hidden_size))
B_stat = np.zeros((Tsize, hidden_size))
C_stat = np.zeros((Tsize, hidden_size))
X_stat = np.zeros((Tsize, input_size))
rates_stat = np.zeros((Tsize, hidden_size))

for xi in xrange(n_train):
    print xi
    x_sample = x_values[xi]
    y_sample = y_hot[xi]

    input_spikes = []
    hidden_spikes = []
    output_spikes = []


    for ti, t in enumerate(np.linspace(T0, T, Tsize)):
        X = poisson(lambda_max * x_sample)
        
        # for n_id in np.where(X)[0]:
        #     input_spikes.append((ti, n_id))

        Xsm = np.zeros(X.shape) #smooth_spikes(t, input_spikes, input_size)
        
        B = np.dot(Xsm, W0)

        dC = - gL * C + gB * (B - C) + gA * (A - C)
        
        C = C + dt * dC

        rates = lambda_max * act(C)
        
        # for b_id, n_id in zip(*np.where(poisson(rates))):
        #     hidden_spikes.append((ti, b_id, n_id))

        # hidden_sm = smooth_spikes_batch(t, hidden_spikes, batch_size, hidden_size)

        C_stat[ti] = C[batch_to_listen]
        # rates_stat[ti] = hidden_sm[batch_to_listen]
        X_stat[ti] = Xsm

