
import numpy as np
from poc.datasets import *
from poc.util import *


def ltd(a, a_mp):
    a_silent_mp = np.where(a_mp < 1e-10)
    a_ltd = np.zeros((a.shape))
    a_ltd[a_silent_mp] = a[a_silent_mp]
    return a_ltd

sigmoid = lambda x: 1.0/(1.0 + np.exp(-x))
def sigmoid_prime(x):
    v = sigmoid(x)
    return v * (1.0 - v)

relu = lambda x: np.maximum(x, 0.0)
def relu_prime(x):
    dadx = np.zeros(x.shape)
    # dadx[np.where(x > 0.0)] = 1.0
    a = relu(x)
    dadx[np.where(a > 0.0)] = 1.0
    return dadx

linear = lambda x: x
linear_prime = lambda x: 1.0

x = np.asarray([
    [0.0, 0.0],
    [0.0, 1.0],
    [1.0, 0.0],
    [1.0, 1.0]
], dtype=np.float32)
y = one_hot_encode(np.asarray([
    [0.0],
    [1.0],
    [1.0],
    [0.0]
], dtype=np.float32), 2)

f, fprime = linear, linear_prime


input_size = 2
layer_size = 50
output_size = 2

w_sd = 0.1
w_b = 0.0

W0 = w_sd * (np.random.random((input_size, layer_size)) - w_b)
W1 = w_sd * (np.random.random((layer_size, layer_size)) - w_b)
W2 = w_sd * (np.random.random((layer_size, output_size)) - w_b)



u0 = np.dot(x, W0)
a0 = f(u0)

u1 = np.dot(a0, W1)
a1 = f(u1)

u2 = np.dot(a1, W2)
a2 = f(u2)

a1_mp = f(np.dot(y, W2.T))

du1_mp = (a1_mp - ltd(a1, a1_mp))

# du1_mp = (a1_mp - ltd(a1, a1_mp)) * fprime(u1)

a0_mp = f(np.dot(a1_mp, W1.T))
du0_mp = (a0_mp - a0)

de = (y - a2)

a1_bp = np.dot(de, W2.T)
a0_bp = np.dot(a1_bp, W1.T)


