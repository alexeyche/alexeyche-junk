

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

from util import shl, shm
from activation import *
from cost import MseCost
from opt import * 

def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)

def batch_inner(left, right):
    return np.asarray([np.inner(left[i], right[i]) for i in xrange(left.shape[0])])

np.random.seed(1)

Tmax = 1000.0
dt = 1.0
filter_size = 50
T0 = filter_size*dt
layer_size = 25
Tsize = int(Tmax/dt)
lambda_max = 200.0/1000.0      # 1/ms
int_time = 20.0
alpha = dt/int_time
alpha_long = dt/(2.0 * Tmax)
gL = 0.1
gD = 0.6
gB = 0.6
gA = 0.6
kB = gB/(gL + gB + gA)


T = np.linspace(T0, Tmax, Tsize)

vol = .10
lag = 10
df = pd.DataFrame(np.random.randn(Tsize+lag-1) * np.sqrt(vol) * np.sqrt(1 / 252.)).cumsum()
x_vec = np.squeeze(df.rolling(lag).mean().dropna().values.copy())
x_vec = x_vec-np.mean(x_vec)

# x_vec = np.sin(T/10.0)


W = 0.1*(np.random.uniform(size=(filter_size, layer_size)) - 0.5)
b = np.zeros((layer_size,))

Y = 0.1*(np.random.uniform(size=(filter_size, layer_size)) - 0.5)
F = 0.1*(np.random.uniform(size=(layer_size, filter_size)) - 0.5)
act = SigmoidActivation()
cost = MseCost()

x_vec_pad = np.pad(x_vec, (filter_size, filter_size), 'constant')

F_start = F.copy()

rate_test = poisson(np.random.random((Tsize, layer_size))*0.1, dt)

C = np.zeros((layer_size,))

# lrates = [1.0, 1.0, 1e-01, 0.0]
lrates = [1.0, 1.0, 0.01]
params = [W, b, F]

# beta1, beta2, factor = 0.9, 0.99, 1.0
# opt = AdamOpt(
#     parameters=params, 
#     learning_rates=[factor * lr * (1.0 - beta1) * (1.0 - beta2) for lr in lrates],
#     beta1=beta1, beta2=beta2, eps=1e-05
# )

opt = SGDOpt(parameters=params, learning_rates=lrates)



for e in xrange(1):
    x_f_vec = np.zeros((Tsize, layer_size))
    rate_vec = np.zeros((Tsize, layer_size))
    C_vec = np.zeros((Tsize, layer_size))
    real_vec = np.zeros((Tsize, layer_size))
    
    

    rate_m = np.zeros((layer_size,))
    Am = np.zeros((layer_size,))
    dF_m = np.zeros(F.shape)
    Cm = np.zeros(C.shape)
    xm = np.zeros((filter_size,))
    APsp_m = np.zeros((filter_size,))

    x_vec_hat = np.zeros(x_vec_pad.shape)

    real_acc = np.zeros((filter_size, layer_size))

    dW_m = np.zeros(W.shape)
    db_m = np.zeros(b.shape)

    rhythm = np.sin(T/50.0)

    gA_t_vec = np.zeros((Tsize,))
    APsp_vec = np.zeros((Tsize, filter_size))
    ap_vec = np.zeros((Tsize, layer_size))
    apr_vec = np.zeros((Tsize, layer_size))
    deriv_part_vec = np.zeros((Tsize, layer_size))
    x_psp_vec = np.zeros((Tsize, filter_size))

    error_acc = 0.0
    for ti, t in enumerate(T):
        xi = ti + filter_size
        rv = rhythm[ti]
        b_rv = rv/2.0 + 0.5
        
        x = x_vec_pad[(xi-filter_size):xi]
        x_psp_vec[ti] = x

        gA_t = gA * b_rv
        
        gA_t_vec[ti] = gA_t

        B = np.dot(x, W) + b
        
        x_actual = x_vec_hat[(xi-filter_size):xi]
        
        APsp = b_rv * x + (1.0 - b_rv) * x_actual
        
        APsp_vec[ti] = APsp

        A = np.dot(APsp, Y)
        
        C += dt * (- gL * C + gB * (B - C) + gA_t * (A - C))

        rate = lambda_max * act(C)
        real = poisson(rate, dt)
        
        real_acc = np.concatenate([np.expand_dims(real, 0), real_acc[:-1]])

        x_vec_hat[xi] = np.sum(batch_inner(real_acc.T, F))

        error_acc += cost(x_vec_hat[xi], x_vec_pad[xi])
        
        de = cost.grad(x_vec_hat[xi], x_vec_pad[xi])        
        dF = real_acc.T * de

        # dF_m = (1.0-alpha_long) * dF_m + alpha_long * dF
        rate_m = (1.0-alpha) * rate_m + alpha * rate
        Am = (1.0-alpha) * Am + alpha * A
        Cm = (1.0-alpha) * Cm + alpha * C
        xm = (1.0-alpha) * xm + alpha * x
        APsp_m = (1.0-alpha) * APsp_m + alpha * APsp

        apical = act(Am) # -1 forward, +1 target 

        ap_vec[ti] = apical
        apr_vec[ti] = rv*apical

        deriv_part = - rv * kB * apical * act.grad(Cm)
        deriv_part_vec[ti] = deriv_part

        # db_m += 0.001 * deriv_part
        # dW_m += 0.001 * np.outer(xm, deriv_part)
        dF_m += 0.001 * dF

        real_vec[ti] = real
        C_vec[ti] = C
        rate_vec[ti] = rate


    # deriv_party = kB * (alpha_t - alpha_f) * act.grad(Am_f)
    # dY = np.outer(APsp_m_f, deriv_party)

    opt.update(dW_m, db_m, dF_m)

    print "Epoch {}, error {:.3f}".format(e, error_acc/len(T))

# shl(x_vec_hat[100:500], x_vec_pad[100:500], show=False)
# shm(real_vec[:300])