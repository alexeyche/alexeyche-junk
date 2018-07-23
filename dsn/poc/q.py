
import numpy as np
from poc.datasets import *
from poc.util import *


def safe_log(x):
    return np.log(x + 1e-05)


def ltd(a, a_mp):
    a_silent_mp = np.where(a_mp < 1e-10)
    a_ltd = np.zeros((a.shape))
    a_ltd[a_silent_mp] = a[a_silent_mp]
    return a_ltd

sigmoid = lambda x: 1.0/(1.0 + np.exp(-x))
def sigmoid_prime(x):
    v = sigmoid(x)
    return v * (1.0 - v)

sigmoid_inv = lambda x: np.log(x/(1.0 - x + 1e-09) + 1e-09)

relu = lambda x: np.maximum(x, 0.0)
def relu_prime(x):
    dadx = np.zeros(x.shape)
    # dadx[np.where(x > 0.0)] = 1.0
    a = relu(x)
    dadx[np.where(a > 0.0)] = 1.0
    return dadx

threshold_value = 0.7
def threshold(x):
    a = np.zeros(x.shape)
    a[np.where(x >= threshold_value)] = 1.0
    return a

threshold_prime = lambda x: 1.0/np.square(1.0 + np.abs(x - threshold_value))
linear = lambda x: x
linear_prime = lambda x: 1.0

# x = np.asarray([
#     [0.0, 0.0],
#     [0.0, 1.0],
#     [1.0, 0.0],
#     [1.0, 1.0]
# ], dtype=np.float32)
# y = one_hot_encode(np.asarray([
#     [0.0],
#     [1.0],
#     [1.0],
#     [0.0]
# ], dtype=np.float32), 2)




# f, fprime, finv = sigmoid, sigmoid_prime, sigmoid_inv
f, fprime, finv = relu, relu_prime, sigmoid_inv

# f, fprime = threshold, threshold_prime

# f, fprime = linear, linear_prime


x = f(np.random.random((4, 20)))
y = f(np.dot(x, np.random.random((20, 10)))/4.0)

input_size = x.shape[1]
layer_size = 30
output_size = y.shape[1]
batch_size = x.shape[0]

wf = 0.01
#
# W0 = wf * (np.random.random((input_size, layer_size)) - 0.5)
# W1 = wf * (np.random.random((layer_size, layer_size)) - 0.5)
# W2 = wf * (np.random.random((layer_size, output_size)) - 0.5)
#

# W0 = random_pos_orth((input_size, layer_size))
# W1 = random_pos_orth((layer_size, layer_size))
# W2 = random_pos_orth((layer_size, output_size))

W0 = random_pos_sparse((input_size, layer_size), p=0.95)
W1 = random_pos_sparse((layer_size, layer_size), p=0.95)
W2 = random_pos_sparse((layer_size, output_size), p=0.95)


u0 = np.dot(x, W0)
a0 = f(u0)

u1 = np.dot(a0, W1)
a1 = f(u1)

u2 = np.dot(a1, W2)
a2 = f(u2)


du2 = y - a2
du1 = np.dot(du2, W2.T) #* fprime(u1)
du0 = np.dot(du1, W1.T) #* fprime(u0)


a2_fb = y
a1_fb = f(np.dot(a2_fb, W2.T))
a0_fb = f(np.dot(a1_fb, W1.T))


# print(np.linalg.norm(y - f(np.dot(np.dot(finv(y), W2.T), W2))))

