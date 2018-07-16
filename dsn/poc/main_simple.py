
import numpy as np
from poc.datasets import *
from poc.util import *


sigmoid = lambda x: 1.0/(1.0 + np.exp(-x))
def sigmoid_prime(x):
    v = sigmoid(x)
    return v * (1.0 - v)

relu = lambda x: np.maximum(x, 0.0)
def relu_prime(x):
    dadx = np.zeros(x.shape)
    dadx[np.where(x > 0.0)] = 1.0
    return dadx

linear = lambda x: x
linear_prime = lambda x: 1.0


f, fprime = relu, relu_prime
# f, fprime = sigmoid, sigmoid_prime
# f, fprime = linear, linear_prime

np.random.seed(12)
batch_size = 200

sd = 0.2
x = np.concatenate([
    np.random.randn(batch_size, 2).astype(np.float32),
    [0.0, 1.0] + sd*np.random.randn(200, 2).astype(np.float32),
    [1.0, 0.0] + sd*np.random.randn(200, 2).astype(np.float32),
    [1.0, 1.0] + sd*np.random.randn(200, 2).astype(np.float32),
], )
y = one_hot_encode(np.concatenate([
    np.zeros((200,), dtype=np.float32),
    np.ones((200,), dtype=np.float32),
    np.ones((200,), dtype=np.float32),
    np.zeros((200,), dtype=np.float32),
]), 2)

ids = np.random.permutation(x.shape[0])
x = x[ids]
y = y[ids]

test_prop = x.shape[0] // 4

xt = x[:test_prop]
yt = y[:test_prop]
x = x[test_prop:]
y = y[test_prop:]


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



input_size = 2
layer_size = 50
output_size = 2

W0 = 0.1 * (np.random.random((input_size, layer_size)) - 0.5)
W1 = 0.1 * (np.random.random((layer_size, output_size)) - 0.5)
b0 = 0.1 * (np.random.random((layer_size,)) - 0.5)
b1 = 0.1 * (np.random.random((output_size,)) - 0.5)

lrate = 0.0001
# lrate = 1.0  # sigmoid

for epoch in range(1):
    u0 = np.dot(x, W0) + b0
    a0 = f(u0)

    u1 = np.dot(a0, W1) + b1
    a1 = f(u1)

    de = (y - a1)

    du0 = np.dot(de, W1.T) * fprime(u0)

    a0_mp = f(np.dot(y, W1.T))

    a0_silent_mp = np.where(a0_mp < 1e-10)
    a0_ltd = np.zeros((a0.shape))
    a0_ltd[a0_silent_mp] = a0[a0_silent_mp]

    du0_mp = (a0_mp - a0_ltd) * fprime(u0)

    # du0 = du0_mp

    dW1 = np.dot(a0.T, de)
    db1 = np.sum(de, 0)

    dW0 = np.dot(x.T, du0)
    db0 = np.sum(du0, 0)


    W0 += lrate * dW0
    W1 += lrate * dW1
    b0 += lrate * db0
    b1 += lrate * db1


    ut0 = np.dot(xt, W0) + b0
    at0 = f(ut0)

    ut1 = np.dot(at0, W1) + b1
    at1 = f(ut1)

    err_rate = np.mean(np.not_equal(np.argmax(at1, 1), np.argmax(yt, 1)))

    if epoch % 100 == 0:
        print("{} {:.4f} {:.4f}".format(epoch, np.linalg.norm(de), err_rate))
