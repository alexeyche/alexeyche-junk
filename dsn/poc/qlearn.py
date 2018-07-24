
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

np.random.seed(11)

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
# f, fprime, finv = relu, relu_prime, sigmoid_inv

f, fprime = threshold, threshold_prime

# f, fprime = linear, linear_prime

# np.random.seed(10)

x = f(np.random.random((10, 20)))
xl = np.dot(x, np.random.random((20, 10)))/5.0
y = f(xl)

input_size = x.shape[1]
layer_size = 100
output_size = y.shape[1]
batch_size = x.shape[0]

wf = 0.01

# W0 = wf * (np.random.random((input_size, layer_size)) - 0.5)
# W1 = wf * (np.random.random((layer_size, layer_size)) - 0.5)
# W2 = wf * (np.random.random((layer_size, output_size)) - 0.5)
#

# W0 = random_orth((input_size, layer_size))
# W1 = random_orth((layer_size, layer_size))
# W2 = random_orth((layer_size, output_size))

# W0 = random_pos_sparse((input_size, layer_size), p=0.1)
# W1 = random_pos_sparse((layer_size, layer_size), p=0.1)
# W2 = random_pos_sparse((layer_size, output_size), p=0.1)

W0 = np.random.random((input_size, layer_size),) - 0.5
W1 = np.random.random((layer_size, layer_size),) - 0.5
W2 = np.random.random((layer_size, output_size),) - 0.5

noise = 0.0
epochs = 1000
metrics = np.zeros((epochs,1))
for epoch in range(epochs):
    u0 = np.dot(x, W0) + np.random.random((batch_size, layer_size))*noise
    a0 = f(u0)

    u1 = np.dot(a0, W1) + np.random.random((batch_size, layer_size))*noise
    a1 = f(u1)

    u2 = np.dot(a1, W2) + np.random.random((batch_size, output_size))*noise
    a2 = f(u2)


    du2 = y - a2
    du1 = np.dot(du2, W2.T) * fprime(u1)
    du0 = np.dot(du1, W1.T) * fprime(u0)


    a2_fb = y
    a1_fb = f(np.dot(a2_fb, W2.T))
    a0_fb = f(np.dot(a1_fb, W1.T))

    du2_fb = y - a2
    du1_fb = (a1_fb - a1) * fprime(u1)
    du0_fb = (a0_fb - a0) * fprime(u0)


    dW0 = np.dot(x.T, du0_fb)
    dW1 = np.dot(a0.T, du1_fb)
    dW2 = np.dot(a1.T, du2_fb)

    W0 += 0.2 * dW0
    W1 += 0.2 * dW1
    W2 += 0.2 * dW2

    metrics[epoch] = (np.linalg.norm(du2), )
    if epoch % 100 == 0:
        print("{} {:.4f} {:.4f} {:.4f} {:.4f}".format(
            epoch,
            np.linalg.norm(du2),
            np.linalg.norm(f(np.dot(x, W0)) - a0_fb),
            np.linalg.norm(f(np.dot(a0_fb, W1)) - a1_fb),
            np.linalg.norm(f(np.dot(a1_fb, W2)) - a2_fb),
        ))

# shl(metrics)
# W1c = W1.copy()
# W1[np.where(np.random.random(W1.shape) < 0.05)] = 0.5
# shm(f(np.dot(np.dot(a0, W1), W1.T)), a0)