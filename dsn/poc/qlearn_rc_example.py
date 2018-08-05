
import numpy as np
from poc.datasets import *
from poc.util import *

from poc.common import *


np.random.seed(11)

def relu_rc(x, W, dt=0.01, num_iters=50):
    u = np.zeros((x.shape[0], W.shape[1]))
    a = np.zeros((x.shape[0], W.shape[1]))
    ah = np.zeros((num_iters, x.shape[0], W.shape[1]))
    uh = np.zeros((num_iters, x.shape[0], W.shape[1]))
    for idx in range(num_iters):
        du = np.dot(x - np.dot(a, W.T), W)
        u = u + dt * du
        a = threshold(u)
        ah[idx] = a.copy()
        uh[idx] = u.copy()
    return a, ah, uh


x = (np.random.random((20, 20)) < 0.1).astype(np.float32)
y = (np.random.random((20, 10)) < 0.1).astype(np.float32)

input_size = x.shape[1]
layer_size = 20
output_size = y.shape[1]
batch_size = x.shape[0]

wf = 0.01

W0 = 0.1*np.random.random((input_size, layer_size),)

a, ah, uh = relu_rc(x, W0)
xt, xt_h, xut_h = relu_rc(a, W0.T)

