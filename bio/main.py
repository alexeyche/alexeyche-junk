import gc
import numpy as np
from util import shl, shm, shs
from functools import partial
from activation import *
from datasets import get_toy_data, quantize_data
import time
import weave
from sklearn.metrics import log_loss        
from opt import * 

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

gL = 0.1
gD = 0.6
gB = 0.6
gA = 0.6

kB = gB/(gL + gB + gA)
kD = gD/(gL + gD)

T0, T = 0.0, 50.0      # ms
dt = 1.0                # ms

int_time = 20.0
alpha = dt/int_time

Ee = 8.0
Ei = -8.0

poisson = partial(poisson, dt=dt)

Tsize = int(T/dt + dt)

lambda_max = 200.0/1000.0      # 1/ms

lr1 = 20.0/lambda_max
lr0 = 20.0/(lambda_max * lambda_max)

# act = SoftplusActivation()
act = SigmoidActivation()

x_values, y_values = get_toy_data()
n_train = (4 * x_values.shape[0]/5)
n_valid = (1 * x_values.shape[0]/5)

y_uniq = np.unique(y_values)
y_hot = one_hot(y_values, len(y_uniq))


input_size = x_values.shape[1]
input_len = x_values.shape[0]
hidden_size = 20
output_size = len(y_uniq)


batch_size = 400
n_train_batches = n_train/batch_size
n_valid_batches = n_valid/batch_size
batch_to_listen = 0

L, M, N = input_size, hidden_size, output_size

#######################################################################

x_values_sm = smooth_samples(x_values, Tsize, lambda_max, koeff_epsp, tau_s, tau_l)


class RiseAndDecaySynapse(object):
    def __init__(self, batch_size, layer_size):
        self.Act = np.zeros((batch_size, layer_size))
        self.Act_aux = np.zeros((batch_size, layer_size))
        self.epsp_bias = 1.5
        
    def update(self, x):
        self.Act_aux += - dt * self.Act_aux/tau_s + koeff_epsp * x 
        self.Act += - dt * self.Act/tau_l + self.epsp_bias * koeff_epsp * self.Act_aux 

    @property
    def output(self):
        return self.Act

    def reset(self):
        self.Act_aux = np.zeros(self.Act_aux.shape)
        self.Act = np.zeros(self.Act.shape)

class DecaySynapse(object):
    def __init__(self, batch_size, layer_size):
        self.Act = np.zeros((batch_size, layer_size))
        
    def update(self, x):
        self.Act += - dt * self.Act/tau_l + (1.0/tau_l) * x

    @property
    def output(self):
        return self.Act

    def reset(self):
        self.Act = np.zeros(self.Act.shape)

synapse_class = DecaySynapse

class HiddenLayer(object):
    def __init__(self, batch_size, input_size, layer_size, output_size):
        self.C = np.zeros((batch_size, layer_size))
        self.C_aux = np.zeros((batch_size, layer_size))
        
        self.Chist = np.zeros((Tsize*2, batch_size, layer_size))
        self.Crate_hist = np.zeros((Tsize*2, batch_size, layer_size))
        self.spikes_hist = np.zeros((Tsize*2, batch_size, layer_size))
        self.Ahist = np.zeros((Tsize*2, batch_size, layer_size))

        self.syn = synapse_class(batch_size, layer_size)

        self.W = 0.1*(np.random.uniform(size=(input_size, layer_size)) - 0.5)
        self.b = np.zeros((layer_size,))
        self.Y = np.random.uniform(size=(output_size, layer_size)) - 0.5
        
        self.Crate_m = np.zeros((batch_size, layer_size))
        self.Psp_m = np.zeros((batch_size, input_size))
        self.Cm = np.zeros((batch_size, layer_size))
        self.Am = np.zeros((batch_size, layer_size))
        self.APsp_m = np.zeros((batch_size, output_size))

    def update(self, ti, x, output_activity, target=False):
        hi = ti if not target else Tsize + ti
        B = np.dot(x, self.W) + self.b
        
        A = np.dot(output_activity, self.Y)
        
        self.Ahist[hi] = A

        gA_t = gA if target else 0.0
        
        self.C += dt * (- gL * self.C + gB * (B - self.C) + gA_t * (A - self.C))
        # self.C += dt * (self.C - np.power(self.C, 3.0)/3.0 - self.C_aux + gB * (B - self.C) + gA_t * (A - self.C))
        # self.C_aux += dt * (0.08 * (self.C  - 0.8 * self.C_aux))

        self.Chist[hi] = self.C

        rate = lambda_max * act(self.C)
        self.Crate_hist[hi] = rate

        spikes = poisson(rate)
        self.spikes_hist[hi] = spikes

        self.syn.update(spikes)
        
        self.Cm = (1.0-alpha) * self.Cm + alpha * self.C
        self.Crate_m = (1.0-alpha) * self.Crate_m + alpha * rate
        self.Psp_m = (1.0-alpha) * self.Psp_m + alpha * x
        self.Am = (1.0-alpha) * self.Am + alpha * A
        self.APsp_m = (1.0 - alpha) * self.APsp_m + alpha * output_activity

    def reset(self):
        self.C = np.zeros(self.C.shape)
        self.Cm = np.zeros(self.Cm.shape)
        self.Crate_m = np.zeros(self.Crate_m.shape)
        self.Psp_m = np.zeros(self.Psp_m.shape)
        self.Am = np.zeros(self.Am.shape)
        self.APsp_m = np.zeros(self.APsp_m.shape)
        self.syn.reset()

class OutputLayer(object):
    def __init__(self, batch_size, input_size, layer_size):
        self.U = np.zeros((batch_size, layer_size))

        self.Uhist = np.zeros((Tsize*2, batch_size, layer_size))
        self.Urate_hist = np.zeros((Tsize*2, batch_size, layer_size))
        self.spikes_hist = np.zeros((Tsize*2, batch_size, layer_size))

        self.syn = synapse_class(batch_size, layer_size)
        
        self.Urate = np.zeros((batch_size, layer_size))
        self.W = 0.1*(np.random.uniform(size=(input_size, layer_size)) - 0.5)
        self.b = np.zeros((layer_size,))

        self.Psp_m = np.zeros((batch_size, hidden_size))
        self.Um = np.zeros((batch_size, layer_size))
        self.Urate_m = np.zeros((batch_size, layer_size))
        
    def update(self, ti, x, target, gE, gI):
        hi = ti if not target else Tsize + ti
        
        V = np.dot(x, self.W) + self.b
        
        I = gE * (Ee - self.U) + gI * (Ei - self.U) if target else 0.0
        
        self.U += dt * (- gL * self.U + gD * (V - self.U) + I)
        self.Uhist[hi] = self.U

        rate = lambda_max * act(self.U)
        self.Urate_hist[hi] = rate

        spikes = poisson(rate)
        self.spikes_hist[hi] = spikes
        
        self.syn.update(spikes)

        self.Um = (1.0 - alpha) * self.Um + alpha * self.U
        self.Urate_m =  (1.0 - alpha) * self.Urate_m + alpha * rate
        self.Psp_m = (1.0 - alpha) * self.Psp_m + alpha * x
        
    def reset(self):
        self.U = np.zeros(self.U.shape)
        self.Psp_m = np.zeros(self.Psp_m.shape)
        self.Um = np.zeros(self.Um.shape)
        self.Urate_m = np.zeros(self.Urate_m.shape)
        self.syn.reset()

hidden = HiddenLayer(batch_size, input_size, hidden_size, output_size)
output = OutputLayer(batch_size, hidden_size, output_size)

Y_start = hidden.Y.copy()

lrates = [lr0, lr0, lr0, lr1, lr1]

opt = SGDOpt([hidden.W, hidden.b, hidden.Y, output.W, output.b], lrates)

# beta1, beta2, factor = 0.99, 0.999, 10.0
# opt = AdamOpt(
#     [hidden.W, hidden.b, hidden.Y, output.W, output.b],
#     [ factor * lr * (1.0 - beta1) * (1.0 - beta2) for lr in lrates], 
#     beta1=beta1, beta2=beta2, eps=1e-05
# )


def step(ti, x, target=False, gE=0.0, gI=0.0):
    hidden.update(ti, x, output.syn.output, target=target)
    output.update(ti, hidden.syn.output, target=target, gE=gE, gI=gI)

for e in xrange(100):
    train_error_rate, train_log_loss, train_hidden_error = 0.0, 0.0, 0.0
    
    start_time = time.time()

    for b_id in xrange(n_train_batches):
        l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)
        y = y_hot[l_id:r_id]
        
        gE = y
        gI = 1.0 - y

        for ti, t in enumerate(np.linspace(T0, T, Tsize)):
            x = x_values_sm[ti, l_id:r_id, :]
            step(ti, x)
            
        Cm_f = hidden.Cm.copy()
        Crate_m_f = hidden.Crate_m.copy()
        BPsp_m_f = hidden.Psp_m.copy()
        Am_f = hidden.Am.copy()
        APsp_m_f = hidden.APsp_m.copy()

        Um_f = output.Um.copy()
        Urate_m_f = output.Urate_m.copy()
        VPsp_m_f = output.Psp_m.copy()
        
        for ti, t in enumerate(np.linspace(T0, T, Tsize)):
            x = x_values_sm[ti, l_id:r_id, :]
            step(ti, x, target=True, gE=gE, gI=gI)
            
        Cm_t = hidden.Cm.copy()
        Crate_m_t = hidden.Crate_m.copy()
        BPsp_m_t = hidden.Psp_m.copy()
        Am_t = hidden.Am.copy()
        Um_t = output.Um.copy()
        Urate_m_t = output.Urate_m.copy()
        VPsp_m_t = output.Psp_m.copy()
        
        ########################################

        alpha_f = act(Am_f)
        alpha_t = act(Am_t)

        deriv_part1 = - kD * (Urate_m_t - Urate_m_f) * act.grad(Um_f)
        db1 = np.mean(deriv_part1, 0)
        dW1 = np.mean(batch_outer(VPsp_m_f, deriv_part1), 0)

        deriv_part0 = - kB * (alpha_t - alpha_f) * act.grad(Cm_f)
        db0 = np.mean(deriv_part0, 0)   
        dW0 = np.mean(batch_outer(BPsp_m_f, deriv_part0), 0)

        deriv_party = kB * (alpha_t - alpha_f) * act.grad(Am_f)
        dY = np.mean(batch_outer(APsp_m_f, deriv_party), 0)
        
        opt.update(
            dW0,
            db0,
            dY,
            dW1,
            db1,
        )

        ########################################

        train_log_loss += log_loss(y_hot[l_id:r_id], Urate_m_f/lambda_max)
        train_error_rate += np.mean(np.argmax(Urate_m_f, 1) != y_values[l_id:r_id])
        train_hidden_error += 100.0 * np.mean((alpha_t - alpha_f) ** 2)


        # shm(hidden.Ahist[:, 0, :], file="/home/alexeyche/tmp/{}_A.png".format(e))
        # shl(hidden.Crate_hist[:Tsize,0,:], show=False)
        # shl(output.Urate_hist[:Tsize,0,:], show=False)
        # shm(hidden.spikes_hist[:Tsize,0,:], show=False)
        # shm(output.spikes_hist[:Tsize,0,:])
        # break
    # break

    valid_error_rate, valid_log_loss = 0.0, 0.0
    
    for b_id in xrange(n_valid_batches):
        l_id, r_id = ((n_train_batches + b_id)*batch_size), ((n_train_batches + b_id+1)*batch_size)

        for ti, t in enumerate(np.linspace(T0, T, Tsize)):
            x = x_values_sm[ti, l_id:r_id, :]

            step(ti, x)

        lambda_U_f = output.Urate_m

        valid_log_loss += log_loss(y_hot[l_id:r_id], lambda_U_f/lambda_max)
        valid_error_rate += np.mean(np.argmax(lambda_U_f, 1) != y_values[l_id:r_id])

    hidden.reset()
    output.reset()
        
    end_time = time.time() 

    print "Epoch {}, ({:.2f}s), train error {:.3f}, train ll {:.3f}, hidden error {:.3f}, test error {:.3f}, test ll {:.3f}".format(
        e, 
        end_time-start_time, 
        train_error_rate/n_train_batches, 
        train_log_loss/n_train_batches, 
        train_hidden_error/n_train_batches, 
        valid_error_rate/n_valid_batches, 
        valid_log_loss/n_valid_batches
    )

# print (time.time() - start)/n_train

# shl(C_stat_t, figsize=(7,7), title="Target", show=False)
# shl(C_stat_f, figsize=(7,7), title="Forward", show=False)
# shl(C_stat_t - C_stat_f, figsize=(7,7), title="Diff")