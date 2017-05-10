import gc
import numpy as np
from util import shl, shm, shs
from functools import partial
from activation import *
from datasets import get_toy_data, quantize_data
import time
import weave
        


def C_step(dt, C, x, act_o, gA, gL,  gB, Y, W0):
    code = """
    int batch_size = NC[0];
    int hidden_size = NC[1];
    int input_size = Nx[0];
    int output_size = Nact_o[1];

    for (int n_id=0; n_id<hidden_size; n_id++) {
        double input_pressure = 0.0;
        for (int inp_id=0; inp_id<input_size; inp_id++) {
            input_pressure +=  x(inp_id) * W0(inp_id, n_id);
        }

        for (int b_id=0; b_id<batch_size; b_id++) {
            double output_pressure = 0.0;
            for (int out_id=0; out_id<output_size; out_id++) {
                output_pressure += act_o(b_id, out_id) * Y(out_id, n_id);
            }

            C(b_id, n_id) += dt * (- gL * C(b_id, n_id) + gB * (input_pressure - C(b_id, n_id)) + gA * (output_pressure - C(b_id, n_id)));
        }
    }
    """
    weave.inline(code,
        ['dt', 'C', 'x', 'act_o', 'gA', 'gL',  'gB', 'Y', 'W0'],
        type_converters=weave.converters.blitz,
        compiler = 'gcc')

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


def smooth_samples(x_values, Tsize, lambda_max, koeff_epsp, tau_s, tau_l):
    n_samples = x_values.shape[0]
    input_size = x_values.shape[1]
    
    x_values_smooth = np.zeros((Tsize, n_samples, input_size), dtype=np.float32)
    
    x_values_smooth_state = np.zeros((n_samples, input_size), dtype=np.float32)
    x_values_smooth_aux = np.zeros((n_samples, input_size), dtype=np.float32)
    
    for ti, t in enumerate(np.linspace(T0, T, Tsize)):
        x_realisation = poisson(lambda_max * x_values)
        
        x_values_smooth_aux += koeff_epsp * x_realisation
        x_values_smooth_state += 1.5 * koeff_epsp * x_values_smooth_aux

        x_values_smooth[ti, :, :] = x_values_smooth_state
        
        x_values_smooth_state -= dt * x_values_smooth_state/tau_l
        x_values_smooth_aux -= dt * x_values_smooth_aux/tau_s

        
    del x_values_smooth_aux, x_values_smooth_state

    gc.collect()
    return x_values_smooth

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
gA = 0.6

T0, T = 0.0, 50.0      # ms
dt = 1.0                # ms
integration_time = int(20.0/dt)

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


batch_size = 10
batch_to_listen = 0

L, M, N = input_size, hidden_size, output_size

#######################################################################

C = np.zeros((batch_size, hidden_size))
U = np.zeros((batch_size, output_size))         
A_plateau = np.zeros((Tsize, batch_size, hidden_size))


W0 = 0.1*(np.random.uniform(size=(input_size, hidden_size)) - 0.5)
b0 = np.zeros((hidden_size,))
W1 = 0.1*(np.random.uniform(size=(hidden_size, output_size)) - 0.5)
b1 = np.zeros((output_size,))
Y = np.random.uniform(size=(output_size, hidden_size)) - 0.5

C_stat_f = np.zeros((Tsize, hidden_size))
U_stat_f = np.zeros((Tsize, output_size))
C_stat_t = np.zeros((Tsize, hidden_size))
U_stat_t = np.zeros((Tsize, output_size))

act_h = np.zeros((batch_size, hidden_size,))
act_aux_h = np.zeros((batch_size, hidden_size,))

act_o = np.zeros((batch_size, output_size,))
act_aux_o = np.zeros((batch_size, output_size,))

x_values_sm = smooth_samples(x_values, Tsize, lambda_max, koeff_epsp, tau_s, tau_l)

n_train = 100

time_acc = 0.0
start = time.time()

def step(ti, x, target, gE, gI):
    global act_h, act_aux_h, act_o, act_aux_o
    global C, U, A_plateau
    global C_stat, U_stat

    B = np.dot(x, W0) + b0
    A_plateau[ti, :] = np.dot(act_o, Y)

    gA_t = gA if target else 0.0
    
    C += dt * (- gL * C + gB * (B - C) + gA_t * (A_plateau[ti, :] - C))
            
    lambda_C = act(C)
    lambda_C_r = poisson(lambda_max * lambda_C)
    
    act_aux_h += koeff_epsp * lambda_C_r
    act_h += 1.5 * koeff_epsp * act_aux_h

    V = np.dot(act_h, W1) + b1
    
    I = gE * (Ee - U) + gI * (Ei - U) if target else 0.0

    U += dt * (- gL * U + gD * (V - U) + I)

    lambda_U = act(U)
    lambda_U_r = poisson(lambda_max * lambda_U)
    
    act_aux_o += koeff_epsp * lambda_U_r
    act_o += 1.5 * koeff_epsp * act_aux_o

    act_o -= dt * act_o/tau_l
    act_aux_o -= dt * act_aux_o/tau_s
    act_h -= dt * act_h/tau_l
    act_aux_h -= dt * act_aux_h/tau_s

    
def drop_state():
    global act_o, act_aux_o, act_h, act_aux_h, C, U
    act_h = np.zeros((batch_size, hidden_size,))
    act_aux_h = np.zeros((batch_size, hidden_size,))

    act_o = np.zeros((batch_size, output_size,))
    act_aux_o = np.zeros((batch_size, output_size,))
    C = np.zeros((batch_size, hidden_size))
    U = np.zeros((batch_size, output_size))

for xi in xrange(n_train):
    drop_state()
    for ti, t in enumerate(np.linspace(T0, T, Tsize)):

        x = x_values_sm[ti, xi, :]
        
        step(ti, x, target=False, gE=0.0, gI=0.0)
        C_stat_f[ti] = C[batch_to_listen]
        U_stat_f[ti] = act_o[batch_to_listen]

    alpha_forward = act(np.mean(A_plateau[-integration_time:], 0))
    
    drop_state()

    y = y_hot[xi]
    gE = y
    gI = 1.0 - y

    for ti, t in enumerate(np.linspace(T0, T, Tsize)):
        x = x_values_sm[ti, xi, :]
        
        step(ti, x, target=True, gE=gE, gI=gI)
        C_stat_t[ti] = C[batch_to_listen]
        U_stat_t[ti] = act_o[batch_to_listen]

    alpha_target = act(np.mean(A_plateau[-integration_time:], 0))        
    
    break

    # print xi
    
print (time.time() - start)/n_train
