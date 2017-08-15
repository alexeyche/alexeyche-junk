

import numpy as np
from util import shl, shm

def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)

def batch_inner(left, right):
    return np.asarray([np.inner(left[i], right[i]) for i in xrange(left.shape[0])])

def relu(x):
    x[x<0] = 0.0
    return x

np.random.seed(2)

Tmax = 250
dt = 1.0
filter_size = 50
T0 = filter_size*dt
layer_size = 25
Tsize = int(Tmax/dt)
lambda_max = 200.0/1000.0      # 1/ms

T = np.linspace(T0, Tmax, Tsize)
x_vec = np.sin(T/10.0)
x_vec_pad = np.pad(x_vec, (filter_size, filter_size), 'constant')

lrate = 1e-04
epsilon = 1.0
tau = 5.0
gamma = 1000.0
alpha = 1.0 #0.99
lam = 1.0
act = lambda x: 1.0/(1.0 + np.exp(-x))

F = 1.0 * (np.random.uniform(size=(filter_size, layer_size)) - 0.5)

u = np.zeros((layer_size,))
a = np.zeros((layer_size,))

u_vec = np.zeros((Tsize, layer_size))
a_vec = np.zeros((Tsize, layer_size))
u_hat_vec = np.zeros((Tsize, layer_size))
u_hat_vec2 = np.zeros((Tsize+filter_size,))
a_acc = np.zeros((filter_size, layer_size))

Fc = np.dot(F.T, F) - np.eye(layer_size)
for ti, t in enumerate(T):
    xi = ti + filter_size

    x = x_vec_pad[(xi-filter_size):xi]

    b = np.dot(x, F)

    u += epsilon * (b - u - np.dot(Fc, a))/tau
    
    # a_entry = gamma * (u - lam)
    # a = (u - alpha * lam) * act(np.clip(a_entry, -100.0, 100.0))
    
    a = relu(u - lam)   # or just simple
    
    a_acc = np.concatenate([np.expand_dims(a, 0), a_acc[:-1]])

    u_vec[ti] = u
    a_vec[ti] = a
    u_hat_vec[ti] = np.sum(batch_inner(F.T, a_acc.T))/tau
    u_hat_vec2[(xi-filter_size):xi] += np.dot(F, a.T)/tau

shl(u_hat_vec2, x_vec_pad, show=False)
shm(a_vec)
