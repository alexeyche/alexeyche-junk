

import numpy as np
from util import shl, shm
from activation import *
from cost import MseCost

def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)

def batch_inner(left, right):
    return np.asarray([np.inner(left[i], right[i]) for i in xrange(left.shape[0])])


np.random.seed(1)

Tmax = 1000
dt = 1.0
filter_size = 50
T0 = filter_size*dt
layer_size = 25
Tsize = int(Tmax/dt)
lambda_max = 200.0/1000.0      # 1/ms

T = np.linspace(T0, Tmax, Tsize)
x_vec = np.sin(T/10.0)

lrate = 1e-04

W = 0.1*(np.random.uniform(size=(filter_size, layer_size)) - 0.5)
Y = 0.1*(np.random.uniform(size=(layer_size, filter_size)) - 0.5)
act = SigmoidActivation()
cost = MseCost()

x_vec_pad = np.pad(x_vec, (filter_size, filter_size), 'constant')

Y_start = Y.copy()

rate_test = poisson(np.random.random((Tsize, layer_size))*0.1, dt)

for e in xrange(100):
    x_f_vec = np.zeros((Tsize, layer_size))
    rate_vec = np.zeros((Tsize, layer_size))
    real_vec = np.zeros((Tsize, layer_size))
    
    real_acc = np.zeros((filter_size, layer_size))
    x_vec_hat = np.zeros((Tsize,))

    error_acc = 0.0
    for ti, t in enumerate(T):
        xi = ti + filter_size

        x = x_vec_pad[(xi-filter_size):xi]

        x_f = np.dot(x, W)
        rate = lambda_max * act(x_f)
        real = poisson(rate, dt)
        
        # real = rate_test[ti]

        real_acc = np.concatenate([np.expand_dims(real, 0), real_acc[:-1]])

        x_vec_hat[ti] = np.sum(batch_inner(real_acc.T, Y))
        
        error_acc += cost(x_vec_hat[ti], x_vec[ti])

        de = cost.grad(x_vec_hat[ti], x_vec[ti])
        dY = real_acc.T * de

        Y -= lrate * dY

        # x_f_vec[ti] = x_f
        # rate_vec[ti] = rate
        real_vec[ti] = real
    

    print "Epoch {}, error {:.3f}".format(e, error_acc/len(T))

