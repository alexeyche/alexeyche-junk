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

def batch_outer(left, right):
    return np.asarray([np.outer(left[i], right[i]) for i in xrange(left.shape[0])])


tau_s = 3.0  # tau rise
tau_l = 10.0 # tau decay
koeff_epsp = 1.0/(tau_l - tau_s)



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

def smooth_spikes(data):
    x_values_sm = np.zeros(data.shape)

    inp_syn = synapse_class(1, data.shape[1])
    for ti in xrange(Tsize):
        inp_syn.update(np.expand_dims(data[ti], 0))
        x_values_sm[ti] = inp_syn.output
    return x_values_sm

gL = 0.1
gD = 0.6
gB = 0.6
gA = 0.6

kB = gB/(gL + gB + gA)
kD = gD/(gL + gD)

T0, T = 0.0, 1000.0      # ms
dt = 1.0                # ms

int_time = 10.0
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


input_size = 50
output_size = input_size
x_values = np.zeros((Tsize, input_size))
for ti in xrange(0, Tsize, 5):
    x_values[ti, ti % input_size] = 1.0



# np.random.seed(1)
# np.random.random((Tsize, ))


input_size = x_values.shape[1]
input_len = x_values.shape[0]
hidden_size = 20
output_size = input_size


batch_size = 1

L, M, N = input_size, hidden_size, output_size

x_values_sm = smooth_spikes(x_values)

#######################################################################



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

        self.Uhist = np.zeros((Tsize, batch_size, layer_size))
        self.Urate_hist = np.zeros((Tsize, batch_size, layer_size))
        self.spikes_hist = np.zeros((Tsize, batch_size, layer_size))

        self.syn = synapse_class(batch_size, layer_size)
        
        self.Urate = np.zeros((batch_size, layer_size))
        self.W = 0.1*(np.random.uniform(size=(input_size, layer_size)) - 0.5)
        self.b = np.zeros((layer_size,))

        self.Psp_m = np.zeros((batch_size, input_size))
        self.Um = np.zeros((batch_size, layer_size))
        self.Urate_m = np.zeros((batch_size, layer_size))
        
    def update(self, ti, x, rhythm_val, gE, gI):
        b_rv = rhythm_val/2.0 + 0.5

        V = np.dot(x, self.W) + self.b
        
        I = (gE * (Ee - self.U) + gI * (Ei - self.U)) * b_rv
        
        self.U += dt * (- gL * self.U + gD * (V - self.U) + I)
        self.Uhist[ti] = self.U

        rate = lambda_max * act(self.U)
        self.Urate_hist[ti] = rate

        spikes = poisson(rate)
        self.spikes_hist[ti] = spikes
        
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

# hidden = HiddenLayer(batch_size, input_size, hidden_size, output_size)
output = OutputLayer(batch_size, input_size, output_size)


opt = SGDOpt([output.W, output.b], [100.0, 0.0])

# beta1, beta2, factor = 0.99, 0.999, 10.0
# opt = AdamOpt(
#     [ factor * lr * (1.0 - beta1) * (1.0 - beta2) for lr in lrates], 
#     beta1=beta1, beta2=beta2, eps=1e-05
# )


Tvec = np.linspace(T0, T, Tsize)

rhythm = np.zeros(Tsize)
w = 200
sign = 1.0
for ti, t in enumerate(Tvec):

    if ti % w == 0 and ti > 0:
        sign = -sign
    rhythm[ti] = sign


for e in xrange(1):
    # rhythm = np.sin(Tvec/50.0)


    start_time = time.time()

    db1_vec = np.zeros((Tsize, output_size))
    dW1_vec = np.zeros((Tsize, input_size, output_size))

    for ti, t in enumerate(Tvec):
        x = x_values_sm[ti, :]
        rhythm_val = rhythm[ti]

        output.update(ti, x, rhythm_val, gE=x, gI=-x)


        deriv_part1 = - kD * rhythm_val * output.Urate_m * act.grad(output.Um)

        db1 = np.mean(deriv_part1, 0)
        dW1 = np.mean(batch_outer(output.Psp_m, deriv_part1), 0)
        
        db1_vec[ti] = db1
        dW1_vec[ti] = dW1
        if ti == 200:
            break        

    dW1 = np.mean(dW1_vec, 0)
    db1 = np.mean(db1_vec, 0) 
    opt.update(dW1, db1)

    error = np.sum(np.square((x_values_sm - smooth_spikes(np.squeeze(output.spikes_hist)))))


    # output.reset()
            
    end_time = time.time() 

    print "Epoch {}, ({:.2f}s), train error {:.3f}".format(
        e, 
        end_time-start_time, 
        error
    )


# shl(C_stat_t, figsize=(7,7), title="Target", show=False)
# shl(C_stat_f, figsize=(7,7), title="Forward", show=False)
# shl(C_stat_t - C_stat_f, figsize=(7,7), title="Diff")