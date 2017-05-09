
import sys
import os

from util import shl, shm, shs
import numpy as np
from datasets import get_toy_data
from os.path import join as pj

from models import Hopfield, ExpDecayHopfield
from activation import *
from cost import MseCost
from models import initialize_layer


from matplotlib import pyplot as plt

def get_batch(d, idx, batch_size):
    return d[idx*batch_size:(idx+1)*batch_size]

def one_hot(y, y_size):
    y_oh = np.zeros((y.shape[0], y_size))
    y_oh[np.arange(y.shape[0]), y] = 1.0
    return y_oh

x_values, y_values = get_toy_data(seed=2)


tmp_dir = "/home/alexeyche/bm"
# [ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]
#
np.random.seed(8)

input_size = x_values.shape[1]
hidden_size = 100
output_size = len(np.unique(y_values))
batch_size = 100
epsilon = 0.1
beta = 1.0
n_neg = 100
n_pos = 100

net_size = input_size + hidden_size + output_size
sl0, sl1, sl2 = input_size, input_size+hidden_size, input_size+hidden_size+output_size

def make_feed_forward(w):
    w[:sl0, :sl0] = 0.0
    w[sl0:sl1, sl0:sl1] = 0.0
    w[sl1:sl2, sl1:sl2] = 0.0
    w[:sl0, sl1:sl2] = 0.0
    w[sl1:sl2, :sl0] = 0.0
    return w


W = initialize_layer(net_size, net_size)
W = (W + W.T)/2.0
W = make_feed_forward(W)

# W *= 0.1

bias = np.zeros((net_size,))
# bias = np.random.randn(net_size,)




act = ClipActivation()
# act = ExpClipActivation()
# act = SoftplusActivation()
# act = SigmoidActivation()
# act = ExpActivation()

# model = Hopfield(act, W, bias)

model = ExpDecayHopfield(act, W, bias)
cost = MseCost()



n_train = (4 * x_values.shape[0]/5)/batch_size
n_valid = (1 * x_values.shape[0]/5)/batch_size

# shs(x_values, labels=y_values)
n_batches = x_values.shape[0]/batch_size


# u_p = np.zeros((x_values.shape[0], net_size))


index = 1

x_t = get_batch(x_values, index, batch_size=batch_size)
y_idx = get_batch(y_values, index, batch_size=batch_size)
y_t = one_hot(y_idx, output_size)


clip_dx = True

# x0 = np.zeros((batch_size, net_size))
x0 = np.random.random((batch_size, net_size))

x = x0.copy()

x_acc = [x.copy()[:,input_size:]]
dx_acc = []

for it in xrange(n_neg):
    x[:, :input_size] = x_t
    
    dx = model.dynamics(x)
    
    if clip_dx:
        x = np.clip(x - epsilon * dx, 0.0, 1.0)
    else:
        x = x - epsilon * dx
    
    x_acc.append(x.copy()[:,input_size:])
    dx_acc.append(dx.copy()[:,input_size:])

for it in xrange(n_pos):
    x[:, :input_size] = x_t
    
    y_hat = x[:, -output_size:]

    dx = model.dynamics(x)
    
    dx[:, -output_size:] += beta * cost.grad(y_hat, y_t)
        
    if clip_dx:
        x = np.clip(x - epsilon * dx, 0.0, 1.0)
    else:
        x = x - epsilon * dx
    
    x_acc.append(x.copy()[:,input_size:])
    dx_acc.append(dx.copy()[:,input_size:])



x_acc = np.squeeze(np.asarray(x_acc))
dx_acc = np.squeeze(np.asarray(dx_acc))

plt.figure()
plt.subplot(2, 3, 1)
plt.plot(x_acc[:,0,:])

plt.subplot(2, 3, 2)
plt.plot(x_acc[:,1,:])

plt.subplot(2, 3, 3)
plt.plot(x_acc[:,2,:])


plt.subplot(2, 3, 4)
plt.plot(dx_acc[:,0,:])


plt.subplot(2, 3, 5)
plt.plot(dx_acc[:,1,:])


plt.subplot(2, 3, 6)
plt.plot(dx_acc[:,2,:])


x_neg_end = x_acc[n_neg-1,:][:,:hidden_size]

plt.figure()

plt.subplot(2, 1, 1)
plt.imshow(x_neg_end.T, cmap='gray', origin='lower')
plt.colorbar()

x_pos_end = x_acc[-1,:][:,:hidden_size]
plt.subplot(2, 1, 2)
plt.imshow(x_pos_end.T, cmap='gray', origin='lower')
plt.colorbar()


plt.suptitle("Diff: {}".format(np.mean((x_pos_end - x_neg_end) ** 2)))
plt.show()





