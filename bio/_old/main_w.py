import gc
import numpy as np
from util import shl, shm, shs
from functools import partial
from activation import *
from datasets import get_toy_data, quantize_data
import time
import weave
from sklearn.metrics import log_loss        



# def step(x):
#     return 1 * (x >= 0)

# def epsp_kernel(t, tau_s, tau_l):
#     koef = 1.0/(tau_l - tau_s)
#     return step(t) * (np.exp(-t/tau_l) - np.exp(-t/tau_s)) * koef

def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)

def one_hot(y, y_size):
    y_oh = np.zeros((y.shape[0], y_size))
    y_oh[np.arange(y.shape[0]), y] = 1.0
    return y_oh


def batch_outer(left, right):
    return np.asarray([np.outer(left[i], right[i]) for i in xrange(left.shape[0])])


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


koeff_epsp = 1.0/(tau_l - tau_s)
epsp_bias = 1.5

gL = 0.1
gD = 0.6
gB = 0.6
gA = 0.6

kB = gB/(gL + gB + gA)
kD = gD/(gL + gD)

T0, T = 0.0, 50.0      # ms
dt = 1.0                # ms
integration_time = int(20.0/dt)
int_time = int(20.0/dt)

Ee = 8.0
Ei = -8.0

poisson = partial(poisson, dt=dt)

Tsize = int(T/dt + dt)

lambda_max = 200.0/1000.0      # 1/ms

P0 = 20.0/lambda_max
P1 = 20.0/(lambda_max * lambda_max)
# alt
P0 = P1
P1 = P1/10.0

learning_rate = 3 * 0.21

act = SigmoidActivation()

x_values, y_values = get_toy_data(seed=2)
n_train = (4 * x_values.shape[0]/5)
n_valid = (1 * x_values.shape[0]/5)


y_uniq = np.unique(y_values)
y_hot = one_hot(y_values, len(y_uniq))


input_size = x_values.shape[1]
input_len = x_values.shape[0]
hidden_size = 300
output_size = len(y_uniq)


batch_size = 500
n_train_batches = n_train/batch_size
n_valid_batches = n_valid/batch_size
batch_to_listen = 1

L, M, N = input_size, hidden_size, output_size

#######################################################################

C = np.zeros((batch_size, hidden_size))
U = np.zeros((batch_size, output_size))         
A_plateau = np.zeros((Tsize, batch_size, hidden_size))
V_psp_hist = np.zeros((Tsize, batch_size, hidden_size))
B_psp_hist = np.zeros((Tsize, batch_size, input_size))
C_hist = np.zeros((Tsize, batch_size, hidden_size))
U_hist = np.zeros((Tsize, batch_size, output_size))
lambda_U_hist = np.zeros((Tsize, batch_size, output_size))
lambda_C_hist = np.zeros((Tsize, batch_size, hidden_size))
lambda_U_r_hist = np.zeros((Tsize, batch_size, output_size))
lambda_C_r_hist = np.zeros((Tsize, batch_size, hidden_size))

I_hist = np.zeros((Tsize, batch_size, output_size))

W0 = 0.1*(np.random.uniform(size=(input_size, hidden_size)) - 0.5)
W0_start = W0.copy()
# W0 = np.ones((input_size, hidden_size))
b0 = np.zeros((hidden_size,))
W1 = 0.1*(np.random.uniform(size=(hidden_size, output_size)) - 0.5)
W1_start = W1.copy()
# W1 = np.ones((hidden_size, output_size))
b1 = np.zeros((output_size,))
Y = np.random.uniform(size=(output_size, hidden_size)) - 0.5
# Y = np.ones((output_size, hidden_size))


act_h = np.zeros((batch_size, hidden_size,))
act_aux_h = np.zeros((batch_size, hidden_size,))

act_o = np.zeros((batch_size, output_size,))
act_aux_o = np.zeros((batch_size, output_size,))

np.random.seed(10)

x_values_sm = smooth_samples(x_values, Tsize, lambda_max, koeff_epsp, tau_s, tau_l)

time_acc = 0.0
start = time.time()



class HiddenLayer(object):
    def __init__(self, batch_size, input_size, layer_size, output_size):
        self.C = np.zeros((batch_size, layer_size))
        self.Cm = np.zeros((batch_size, layer_size))
        self.Am = np.zeros((batch_size, layer_size))
        self.Crate = np.zeros((batch_size, layer_size))
        self.Cact = np.zeros((batch_size, layer_size))
        self.Cact_aux = np.zeros((batch_size, layer_size))
        self.Bm = np.zeros((batch_size, layer_size))
        self.W = 0.1*(np.random.uniform(size=(input_size, layer_size)) - 0.5)
        self.b = np.zeros((layer_size,))
        self.Y = np.random.uniform(size=(output_size, layer_size)) - 0.5

        self.Chist = np.zeros((Tsize, layer_size))
        self.Crate_hist = np.zeros((Tsize, layer_size))
        self.Cspikes_hist = np.zeros((Tsize, layer_size))
        self.Cact_hist = np.zeros((Tsize, layer_size))



class OutputLayer(object):
    def __init__(self, batch_size, input_size, layer_size):
        self.U = np.zeros((batch_size, layer_size))
        self.Um = np.zeros((batch_size, layer_size))
        self.Uact = np.zeros((batch_size, layer_size))
        self.Uact_aux = np.zeros((batch_size, layer_size))
        self.Urate = np.zeros((batch_size, layer_size))
        self.W = 0.1*(np.random.uniform(size=(input_size, layer_size)) - 0.5)
        self.b = np.zeros((layer_size,))

        self.Ihist = np.zeros((Tsize, layer_size))
        self.Uhist = np.zeros((Tsize, layer_size))
        self.Urate_hist = np.zeros((Tsize, layer_size))



def net_step(ti, x, hidden, output, gE, gI, target):
    support_code = """
    double act(double x) {
        return 1.0/(1.0 + exp(-x));
    }
    double poisson(double r, double dt) {
        double U = (double)rand() / (double)RAND_MAX;
        return (dt * r) > U;
    }
    """
    prep_code = ""
    for name in hidden.names:
        prep_code += r"""
        py::object _{obj}_{name} = {obj}.attr("{name}");
        PyArrayObject* _{obj}_{name}_array = convert_to_numpy(_{obj}_{name},"{name}");
        blitz::Array<double, {dim_size}> {obj}_{name} = convert_to_blitz<double,{dim_size}>(_{obj}_{name}_array,"{name}");
        """.format(obj="hidden", name=name, dim_size=len(getattr(hidden, name).shape))
    
    for name in output.names:
        prep_code += r"""
        py::object _{obj}_{name} = {obj}.attr("{name}");
        PyArrayObject* _{obj}_{name}_array = convert_to_numpy(_{obj}_{name},"{name}");
        blitz::Array<double, {dim_size}> {obj}_{name} = convert_to_blitz<double,{dim_size}>(_{obj}_{name}_array,"{name}");
        """.format(obj="output", name=name, dim_size=len(getattr(output, name).shape))
    

    code = r"""
    int hidden_size = hidden_C.shape()[1];
    int output_size = output_U.shape()[1];
    int input_size = x.shape()[1];
    
    for (int b_id=0; b_id<batch_size; b_id++) {
        for (int n_id=0; n_id<hidden_size; n_id++) {
            double input_pressure = 0.0;
            for (int inp_id=0; inp_id<input_size; inp_id++) {
                input_pressure +=  x(b_id, inp_id) * hidden_W(inp_id, n_id) + hidden_b(n_id);
            }

            hidden_Bm(b_id, n_id) += (- hidden_Bm(b_id, n_id) + input_pressure)/int_time;
            
            double output_pressure = 0.0;
            if (target) {
                for (int o_id=0; o_id < output_size; ++o_id) {
                    output_pressure += output_Uact(b_id, o_id) * hidden_Y(o_id, n_id);
                }
                
            }

            double plateau_potentials = target ? gA * (output_pressure - hidden_C(b_id, n_id)) : 0.0;
            
            hidden_C(b_id, n_id) += dt * (
                - gL * hidden_C(b_id, n_id) + gB * (input_pressure - hidden_C(b_id, n_id)) + plateau_potentials
            );

            hidden_Crate(b_id, n_id) = lambda_max * act(hidden_C(b_id, n_id));
            double spike = poisson(hidden_Crate(b_id, n_id), dt);

            hidden_Cact_aux(b_id, n_id) += - hidden_Cact_aux(b_id, n_id)/tau_s + koeff_epsp * spike;
            hidden_Cact(b_id, n_id) += - hidden_Cact(b_id, n_id)/tau_l + epsp_bias * koeff_epsp * hidden_Cact_aux(b_id, n_id);

            if (b_id == batch_to_listen) {
                hidden_Chist(ti, n_id) = hidden_C(b_id, n_id);
                hidden_Crate_hist(ti, n_id) = hidden_Crate(b_id, n_id);
                hidden_Cspikes_hist(ti, n_id) = spike;
                hidden_Cact_hist(ti, n_id) = hidden_Cact(b_id, n_id);
            }
        }
        for (int n_id=0; n_id<output_size; n_id++) {
            double input_pressure = 0.0;
            for (int inp_id=0; inp_id<hidden_size; inp_id++) {
                input_pressure +=  hidden_Cact(b_id, inp_id) * output_W(inp_id, n_id) + output_b(n_id);
            }

            double I = 0.0;
            if (target) {
                I = gE(b_id, n_id) * (Ee - output_U(b_id, n_id)) + gI(b_id, n_id) * (Ei - output_U(b_id, n_id));
            }
            output_U(b_id, n_id) += dt * (
                - gL * output_U(b_id, n_id) + gD * (input_pressure - output_U(b_id, n_id)) + I
            );

            output_Urate(b_id, n_id) = lambda_max * act(output_U(b_id, n_id));
            double spike = poisson(output_Urate(b_id, n_id), dt);

            output_Uact_aux(b_id, n_id) += - output_Uact_aux(b_id, n_id)/tau_s + koeff_epsp * spike;
            output_Uact(b_id, n_id) += - output_Uact(b_id, n_id)/tau_l + epsp_bias * koeff_epsp * output_Uact_aux(b_id, n_id);

            if (b_id == batch_to_listen) {
                output_Ihist(ti, n_id) = I;
                output_Uhist(ti, n_id) = output_U(b_id, n_id);
                output_Urate_hist(ti, n_id) = output_Urate(b_id, n_id);
            }
        }
    }
    """
    target = int(target)
    weave.inline(
        prep_code + code,
        ['dt', 'gI', 'gE', 'gL', 'gB', 'gA', 'gD', 'int_time', 'batch_to_listen', 'lambda_max'] + 
        ['tau_l', 'tau_s', 'koeff_epsp', 'epsp_bias', 'Ee', 'Ei'] + 
        ['x', 'ti', 'hidden', 'output', 'batch_size', 'target'],
        support_code=support_code,
        type_converters=weave.converters.blitz,
        compiler = 'gcc'
    )


import ctypes

# pointer to float type, for convenience
c_float_p = ctypes.POINTER(ctypes.c_float)

_net_step = ctypes.cdll.LoadLibrary("/Users/aleksei/distr/alexeyche-junk/bio/net_step.so")
_net_step.net_step.argtypes = (
    ctypes.c_uint32, # ti
    ctypes.c_float, # dt
    ctypes.c_bool,  # target
    ctypes.c_float, # gL
    ctypes.c_float, # gB
    ctypes.c_float, # gA
    ctypes.c_float, # gD
    ctypes.c_float, # int_time
    ctypes.c_float, # batch_to_listen
    ctypes.c_float, # lambda_max
    ctypes.c_float, # tau_l
    ctypes.c_float, # tau_s
    ctypes.c_float, # koeff_epsp
    ctypes.c_float, # epsp_bias
    ctypes.c_float, # Ee
    ctypes.c_float, # Ei
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # x
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_C
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Cm
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Am
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Crate
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Cact
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Cact_aux
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Bm
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_W
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Y
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Chist
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Crate_hist
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Cspikes_hist
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # hidden_Cact_hist
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_U
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_Um
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_Uact
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_Uact_aux
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_Urate
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_W
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_Ihist
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_Uhist
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # output_Urate_hist
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # gE
    c_float_p, ctypes.c_uint32, ctypes.c_uint32, # gI
    c_float_p, ctypes.c_uint32, # hidden_b
    c_float_p, ctypes.c_uint32, # output_b
)
net_step_fun = _net_step.net_step

# v = np.ascontiguousarray(np.random.randn(2, 5), dtype=np.float32)

# v = np.asarray([[1.0, 2.0], [3.0, 4.0]], dtype=np.float32)





def step(ti, x, target, gE, gI):
    global act_h, act_aux_h, act_o, act_aux_o
    global C, U, A_plateau
    global C_stat, U_stat
    global B_psp_hist, V_psp_hist, C_hist, lambda_U_hist, lambda_C_hist, I_hist
    
    B_psp_hist[ti] = x

    B = np.dot(x, W0) + b0
    A_plateau[ti] = np.dot(act_o, Y)

    gA_t = gA if target else 0.0
    
    C += dt * (- gL * C + gB * (B - C) + gA_t * (A_plateau[ti] - C))

    C_hist[ti] = C

        
    lambda_C = lambda_max * act(C)
    lambda_C_r = poisson(lambda_C)
    lambda_C_hist[ti] = lambda_C.copy()
    lambda_C_r_hist[ti] = lambda_C_r.copy()
    
    act_aux_h += koeff_epsp * lambda_C_r
    act_h += 1.5 * koeff_epsp * act_aux_h

    V_psp_hist[ti] = act_h
    
    V = np.dot(act_h, W1) + b1
    I = gE * (Ee - U) + gI * (Ei - U) if target else 0.0
    
    I_hist[ti] = I

    U += dt * (- gL * U + gD * (V - U) + I)

    lambda_U_hist[ti] = lambda_max * act(U)
    lambda_U_r = poisson(lambda_U_hist[ti])
    # lambda_U_r_hist[ti] = lambda_U_r.copy()
    

    act_aux_o += koeff_epsp * lambda_U_r
    act_o += 1.5 * koeff_epsp * act_aux_o

    act_o -= dt * act_o/tau_l
    act_aux_o -= dt * act_aux_o/tau_s
    act_h -= dt * act_h/tau_l
    act_aux_h -= dt * act_aux_h/tau_s

hidden = HiddenLayer(batch_size, input_size, hidden_size, output_size)
output = OutputLayer(batch_size, hidden_size, output_size)

W0 = hidden.W
Y = hidden.Y

hidden_C_ptr = hidden.C.ctypes.data_as(c_float_p)
hidden_Cm_ptr = hidden.Cm.ctypes.data_as(c_float_p)
hidden_Am_ptr = hidden.Am.ctypes.data_as(c_float_p)
hidden_Crate_ptr = hidden.Crate.ctypes.data_as(c_float_p)
hidden_Cact_ptr = hidden.Cact.ctypes.data_as(c_float_p)
hidden_Cact_aux_ptr = hidden.Cact_aux.ctypes.data_as(c_float_p)
hidden_Bm_ptr = hidden.Bm.ctypes.data_as(c_float_p)
hidden_W_ptr = hidden.W.ctypes.data_as(c_float_p)
hidden_Y_ptr = hidden.Y.ctypes.data_as(c_float_p)
hidden_Chist_ptr = hidden.Chist.ctypes.data_as(c_float_p)
hidden_Crate_hist_ptr = hidden.Crate_hist.ctypes.data_as(c_float_p)
hidden_Cspikes_hist_ptr = hidden.Cspikes_hist.ctypes.data_as(c_float_p)
hidden_Cact_hist_ptr = hidden.Cact_hist.ctypes.data_as(c_float_p)
output_U_ptr = output.U.ctypes.data_as(c_float_p)
output_Um_ptr = output.Um.ctypes.data_as(c_float_p)
output_Uact_ptr = output.Uact.ctypes.data_as(c_float_p)
output_Uact_aux_ptr = output.Uact_aux.ctypes.data_as(c_float_p)
output_Urate_ptr = output.Urate.ctypes.data_as(c_float_p)
output_W_ptr = output.W.ctypes.data_as(c_float_p)
output_Ihist_ptr = output.Ihist.ctypes.data_as(c_float_p)
output_Uhist_ptr = output.Uhist.ctypes.data_as(c_float_p)
output_Urate_hist_ptr = output.Urate_hist.ctypes.data_as(c_float_p)
hidden_b_ptr = hidden.b.ctypes.data_as(c_float_p)
output_b_ptr = output.b.ctypes.data_as(c_float_p)

def net_step(ti, x, target, gE, gI):
    net_step_fun(
        ti,
        dt,
        target,
        gL,
        gB,
        gA,
        gD,
        int_time,
        batch_to_listen,
        lambda_max,
        tau_l,
        tau_s,
        koeff_epsp,
        epsp_bias,
        Ee,
        Ei,
        x.ctypes.data_as(c_float_p), x.shape[0], x.shape[1],
        hidden_C_ptr, hidden.C.shape[0], hidden.C.shape[1],
        hidden_Cm_ptr, hidden.Cm.shape[0], hidden.Cm.shape[1],
        hidden_Am_ptr, hidden.Am.shape[0], hidden.Am.shape[1],
        hidden_Crate_ptr, hidden.Crate.shape[0], hidden.Crate.shape[1],
        hidden_Cact_ptr, hidden.Cact.shape[0], hidden.Cact.shape[1],
        hidden_Cact_aux_ptr, hidden.Cact_aux.shape[0], hidden.Cact_aux.shape[1],
        hidden_Bm_ptr, hidden.Bm.shape[0], hidden.Bm.shape[1],
        hidden_W_ptr, hidden.W.shape[0], hidden.W.shape[1],
        hidden_Y_ptr, hidden.Y.shape[0], hidden.Y.shape[1],
        hidden_Chist_ptr, hidden.Chist.shape[0], hidden.Chist.shape[1],
        hidden_Crate_hist_ptr, hidden.Crate_hist.shape[0], hidden.Crate_hist.shape[1],
        hidden_Cspikes_hist_ptr, hidden.Cspikes_hist.shape[0], hidden.Cspikes_hist.shape[1],
        hidden_Cact_hist_ptr, hidden.Cact_hist.shape[0], hidden.Cact_hist.shape[1],
        output_U_ptr, output.U.shape[0], output.U.shape[1],
        output_Um_ptr, output.Um.shape[0], output.Um.shape[1],
        output_Uact_ptr, output.Uact.shape[0], output.Uact.shape[1],
        output_Uact_aux_ptr, output.Uact_aux.shape[0], output.Uact_aux.shape[1],
        output_Urate_ptr, output.Urate.shape[0], output.Urate.shape[1],
        output_W_ptr, output.W.shape[0], output.W.shape[1],
        output_Ihist_ptr, output.Ihist.shape[0], output.Ihist.shape[1],
        output_Uhist_ptr, output.Uhist.shape[0], output.Uhist.shape[1],
        output_Urate_hist_ptr, output.Urate_hist.shape[0], output.Urate_hist.shape[1],
        gE.ctypes.data_as(c_float_p), gE.shape[0], gE.shape[1],
        gI.ctypes.data_as(c_float_p), gI.shape[0], gI.shape[1],
        hidden_b_ptr, hidden.b.shape[0],
        output_b_ptr, output.b.shape[0],
    )

def fun():
    for b_id in xrange(n_train_batches):

        l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)

        y = y_hot[l_id:r_id]

        gE = y
        gI = 1.0 - y


        for ti, t in enumerate(np.linspace(T0, T, Tsize)):
            x = x_values_sm[ti, l_id:r_id, :]

            net_step(ti, x, target=True, gE=gE, gI=gI)
            #
            # step(ti, x, target=True, gE=gE, gI=gI)

import cProfile

pf = '/Users/aleksei/tmp/profile'
cProfile.run('fun()', pf)
import pstats
p = pstats.Stats(pf)
p.strip_dirs().sort_stats("tottime").print_stats()    

# # start_time = time.time()
# # for b_id in xrange(n_train_batches):

# #     l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)

# #     y = y_hot[l_id:r_id]

# #     gE = y
# #     gI = 1.0 - y

# #     for ti, t in enumerate(np.linspace(T0, T, Tsize)):
# #         x = x_values_sm[ti, l_id:r_id, :]

# #         # net_step(ti, x, hidden, output, gE, gI, target=True)
        
# #         step(ti, x, target=True, gE=gE, gI=gI)

# # print time.time() - start_time


# # lambda_U_hist[:, batch_to_listen,:]

# # shl(V_psp_hist[:, batch_to_listen, :], show=False)
# # shl(hidden.Cact_hist)
# # shm(hidden.Cspikes_hist, lambda_C_r_hist[:, batch_to_listen, :])


# b_id = 0
# l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)

# y = y_hot[l_id:r_id]

# gE = y
# gI = 1.0 - y

# import cProfile

# def fun():
#     for ti, t in enumerate(np.linspace(T0, T, Tsize)):
#         x = x_values_sm[ti, l_id:r_id, :]
            
#         # net_step(ti, x, hidden, output, gE, gI, target=True)

#         step(ti, x, target=True, gE=gE, gI=gI)
            


# def drop_state():
#     global act_o, act_aux_o, act_h, act_aux_h, C, U
    
#     act_h = np.zeros((batch_size, hidden_size,))
#     act_aux_h = np.zeros((batch_size, hidden_size,))

#     act_o = np.zeros((batch_size, output_size,))
#     act_aux_o = np.zeros((batch_size, output_size,))
#     C = np.zeros((batch_size, hidden_size))
#     U = np.zeros((batch_size, output_size))


# for e in xrange(1000):
#     train_error_rate, train_log_loss = 0.0, 0.0
    
#     start_time = time.time()
    
#     for b_id in xrange(n_train_batches):
#         l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)
#         y = y_hot[l_id:r_id]
        
#         gE = y
#         gI = 1.0 - y

        
#         drop_state()
#         for ti, t in enumerate(np.linspace(T0, T, Tsize)):

#             x = x_values_sm[ti, l_id:r_id, :]
            
#             net_step(ti, x, hidden, output, gE, gI, target=False)

#             # step(ti, x, target=False, gE=0.0, gI=0.0)
            
#         A_plateau_f = A_plateau.copy()
#         V_psp_hist_f = V_psp_hist.copy()
#         B_psp_hist_f = B_psp_hist.copy()
#         C_hist_f = C_hist.copy()
#         U_hist_f = U_hist.copy()
#         lambda_U_hist_f = lambda_U_hist.copy()
        
#         # just for stat
#         lambda_C_hist_f = lambda_C_hist.copy()
#         I_hist_f = I_hist.copy()

#         drop_state()

#         for ti, t in enumerate(np.linspace(T0, T, Tsize)):
#             x = x_values_sm[ti, l_id:r_id, :]
            
#             net_step(ti, x, hidden, output, gE, gI, target=True)
            
#             # step(ti, x, target=True, gE=gE, gI=gI)

#         A_plateau_t = A_plateau.copy()
#         lambda_U_hist_t = lambda_U_hist.copy()
        
            
#         # just for stat
#         V_psp_hist_t = V_psp_hist.copy()
#         B_psp_hist_t = B_psp_hist.copy()
#         C_hist_t = C_hist.copy()
#         U_hist_t = U_hist.copy()
#         lambda_C_hist_t = lambda_C_hist.copy()
#         I_hist_t = I_hist.copy()

#         ########################################


#         V_psp_mean_f = np.mean(V_psp_hist_f[-integration_time:], 0)
#         B_psp_mean_f = np.mean(B_psp_hist_f[-integration_time:], 0)
#         C_mean_f = np.mean(C_hist_f[-integration_time:], 0)
#         U_mean_f = np.mean(U_hist_f[-integration_time:], 0)
#         lambda_U_f = np.mean(lambda_U_hist_f[-integration_time:], 0)
        

#         alpha_forward = act(np.mean(A_plateau_f[-integration_time:], 0))
#         alpha_target = act(np.mean(A_plateau_t[-integration_time:], 0))        
#         lambda_U_target = np.mean(lambda_U_hist_t[-integration_time:], 0)
        

#         deriv_part1 = - kD * (lambda_U_target - lambda_max * act(U_mean_f)) * act.grad(U_mean_f)
#         db1 = np.mean(deriv_part1, 0)
#         dW1 = np.mean(batch_outer(V_psp_mean_f, deriv_part1), 0)

#         W1 -= learning_rate * P1 * dW1
#         b1 -= learning_rate * P1 * db1

#         deriv_part0 = - kB * (alpha_target - alpha_forward) * act.grad(C_mean_f)
#         db0 = np.mean(deriv_part0, 0)   
#         dW0 = np.mean(batch_outer(B_psp_mean_f, deriv_part0), 0)

#         W0 -= learning_rate * P0 * dW0
#         b0 -= learning_rate * P0 * db0
        
#         train_log_loss += log_loss(y_hot[l_id:r_id], lambda_U_f/lambda_max)
#         train_error_rate += np.mean(np.argmax(lambda_U_f, 1) != y_values[l_id:r_id])

#         # print log_loss(y_hot[l_id:r_id], lambda_U_target/lambda_max)

#         # shl(C_hist_f[:,0,:], C_hist_f[:,1,:], show=False)
#         # shl(C_hist_t[:,0,:], C_hist_t[:,1,:])

#         # shl(lambda_C_hist_f[:,0,:], lambda_C_hist_f[:,1,:], show=False)
#         # shl(lambda_C_hist_t[:,0,:], lambda_C_hist_t[:,1,:])
        
#         # shl(V_psp_hist_f[:,0,:], V_psp_hist_f[:,1,:], show=False)
#         # shl(V_psp_hist_t[:,0,:], V_psp_hist_t[:,1,:])
        
#         # shl(I_hist_t[:,0,:], I_hist_t[:,1,:])
#         # shl(A_plateau_t[:,0,:]-C_hist_t[:,0,:], A_plateau_t[:,1,:]-C_hist_t[:,1,:])
    
#     valid_error_rate, valid_log_loss = 0.0, 0.0
    
#     for b_id in xrange(n_valid_batches):
#         l_id, r_id = ((n_train_batches + b_id)*batch_size), ((n_train_batches + b_id+1)*batch_size)

#         drop_state()

#         for ti, t in enumerate(np.linspace(T0, T, Tsize)):
#             x = x_values_sm[ti, l_id:r_id, :]
            
#             net_step(ti, x, hidden, output, gE, gI, target=False)

#             # step(ti, x, target=False, gE=0.0, gI=0.0)
        
#         lambda_U_f = np.mean(lambda_U_hist[-integration_time:], 0)

#         valid_log_loss += log_loss(y_hot[l_id:r_id], lambda_U_f/lambda_max)
#         valid_error_rate += np.mean(np.argmax(lambda_U_f, 1) != y_values[l_id:r_id])

#         # print b0
#         # break        
#     # break
#     end_time = time.time() 
#     print "Epoch {} ({:.3f} s), train error {:.3f}, train ll {:.3f}, test error {:.3f}, test ll {:.3f}".format(
#         e, end_time-start_time, train_error_rate/n_train_batches, train_log_loss/n_train_batches, valid_error_rate/n_valid_batches, valid_log_loss/n_valid_batches
#     )

# print (time.time() - start)/n_train

# shl(C_stat_t, figsize=(7,7), title="Target", show=False)
# shl(C_stat_f, figsize=(7,7), title="Forward", show=False)
# shl(C_stat_t - C_stat_f, figsize=(7,7), title="Diff")