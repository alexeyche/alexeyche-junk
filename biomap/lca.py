

import numpy as np
from util import shl, shm

sigmoid = lambda x: 1.0/(1.0 + np.exp(-x))

def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)

def batch_inner(left, right):
    return np.asarray([np.inner(left[i], right[i]) for i in xrange(left.shape[0])])

def relu(x):
    x[x<0] = 0.0
    return x

def hebb_rule(x, y, w):
    return np.dot(x.T, y)/x.shape[0]


# def oja_rule(x, y, w):
#     assert w.shape[1] == y.shape[0]
    
#     dw = np.zeros(w.shape)
#     for ni in xrange(y.shape[0]):
        
#         dw[:, ni] = x * y[ni] - w[:, ni] * y[ni] * y[ni]
#         # for nj in xrange(y.shape[0]):
#         #     if ni != nj:
#         #         dw[:, ni] -= w[:, nj] * y[nj] * y[nj]
#         # x_v_norm = x - (y_n * w[:, ni].reshape(w.shape[0], 1)).T

#         # dw[:, ni] = np.mean(y_n.reshape((y_n.shape[0], 1)) * x_v_norm , 0)
    
#     return dw

def oja_rule(x, y, w):
    return np.dot(x.T, y) - np.dot(F, np.dot(a.T, a))


# def oja_rule(x, y, w):
#     assert w.shape[1] == y.shape[1]
#     dw = np.zeros(w.shape)
#     for ni in xrange(y.shape[1]):
#         y_n = y[:, ni]
#         x_v_norm = x - (y_n * w[:, ni].reshape(w.shape[0], 1)).T

#         dw[:, ni] = np.mean(y_n.reshape((y_n.shape[0], 1)) * x_v_norm , 0)
    
#     return dw


# def oja_rule(x, y, w):
#     assert w.shape[1] == y.shape[1]    
#     return np.dot(x.T, y) - np.dot(w, np.tile(y * y, (y.shape[1],1)))


def norm(w):
    return np.asarray([w[:,i]/np.sqrt(np.sum(np.square(w[:,i]))) for i in xrange(w.shape[1])]).T

np.random.seed(2)

Tmax = 250
dt = 1.0
filter_size = 50
T0 = filter_size*dt
layer_size = 50
Tsize = int(Tmax/dt)
lambda_max = 200.0/1000.0      # 1/ms

T = np.linspace(T0, Tmax, Tsize)
x_vec = 1.0*np.sin(T/10.0)
x_vec_pad = np.pad(x_vec, (filter_size, filter_size), 'constant')
x_vec_pad = np.expand_dims(x_vec_pad, 1)  # batch_size = 1

lrate = 1e-04
epsilon = 1.0
tau = 5.0
gamma = 1000.0
alpha = 0.99
lam = 1.0
act = sigmoid
batch_size = 1

# F = np.random.randn(filter_size, layer_size)
# F = norm(F)
F = 1.0 * (np.random.uniform(size=(filter_size, layer_size)) - 0.5)
F = norm(F)

u = np.zeros((batch_size, layer_size,))
a = np.zeros((batch_size, layer_size,))

u_vec = np.zeros((Tsize, batch_size, layer_size))
a_vec = np.zeros((Tsize, batch_size, layer_size))
a_acc = np.zeros((filter_size, batch_size, layer_size))


S = lambda x: np.log(1 + np.square(x))
Sd = lambda x: 2.0 * x / (np.square(x) + 1)

fb_vec = np.zeros((Tsize, batch_size, layer_size))


for e in xrange(4):
    x_hat_vec = np.zeros((Tsize+filter_size, batch_size,))

    Fc = np.dot(F.T, F) - np.eye(layer_size)

    dF = np.zeros(F.shape)
    for ti, t in enumerate(T):
        xi = ti + filter_size

        x = x_vec_pad[(xi-filter_size):xi].T

        # u += epsilon * (- u + np.dot(x - np.dot(F, a.T).T, F))/tau
        
        # r = x - np.dot(F, a.T).T
        # u += epsilon * (- u + np.dot(r, F) - 0.1*Sd(a))/tau

        u += epsilon * (- u + np.dot(x, F) - np.dot(a, Fc) )/tau
        
        # a_entry = gamma * (u - lam)
        # a = (u - alpha * lam) * act(np.clip(a_entry, -100.0, 100.0))
        
        a = relu(u - lam)   # or just simple
        
        dF += (1.0/Tmax) * oja_rule(x, a, F)
        
        # dF += (1.0/Tmax) * hebb_rule(x, a, F)

        a_acc = np.concatenate([np.expand_dims(a, 0), a_acc[:-1]])

        u_vec[ti] = u.copy()
        a_vec[ti] = a.copy()
        x_hat_vec[(xi-filter_size):xi] += np.dot(F, a.T)/tau
        fb_vec[ti] = np.dot(a, Fc)

    # shm(dF)    
    F += 0.1 * dF
    # F = norm(F)
    print "Epoch {}, MSE {}".format(e, np.mean(np.square(x_hat_vec[50:-50] - x_vec_pad[50:-100])))

# x_hat_vec = np.pad(x_hat_vec, (filter_size, 0), 'constant')

shl(x_hat_vec, x_vec_pad, show=False)
shm(a_vec)
