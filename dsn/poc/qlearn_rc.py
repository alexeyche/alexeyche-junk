
import numpy as np
from poc.datasets import *
from poc.util import *

from poc.common import *


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

# f, fprime = threshold, threshold_prime

def relu_rc(x, W, dt=0.01):
    u = np.zeros((x.shape[0], W.shape[1]))
    a = np.zeros((x.shape[0], W.shape[1]))
    for _ in range(50):
        du = np.dot(x - np.dot(a, W.T), W)
        u = u + dt * du
        a = threshold(u)

    return a


# f, fprime = linear, linear_prime

# np.random.seed(10)

# x = relu(np.random.random((10, 20)))
# Wt = np.random.random((20, 10))
# y = relu_rc(x, Wt)
x = (np.random.random((20, 20)) < 0.1).astype(np.float32)
y = (np.random.random((20, 10)) < 0.1).astype(np.float32)

input_size = x.shape[1]
layer_size = 20
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

# W0 = random_pos_sparse((input_size, layer_size), p=0.75)
# W1 = random_pos_sparse((layer_size, layer_size), p=0.75)
# W2 = random_pos_sparse((layer_size, output_size), p=0.75)

W0 = norm(np.random.random((input_size, layer_size),) -0.5)
W1 = norm(np.random.random((layer_size, layer_size),) -0.5)
W2 = norm(np.random.random((layer_size, output_size),)-0.5)

noise = 0.0
epochs = 1000
metrics = np.zeros((epochs,1))

for epoch in range(epochs):
    a0 = relu_rc(x, W0)
    a1 = relu_rc(a0, W1)
    a2 = relu_rc(a1, W2)

    a2_fb = y
    a1_fb = relu_rc(a2_fb, W2.T)
    a0_fb = relu_rc(a1_fb, W1.T)

    du2_fb = y - a2
    du1_fb = (a1_fb - a1)
    du0_fb = (a0_fb - a0)

    dW0 = np.dot(x.T, du0_fb)
    dW1 = np.dot(a0.T, du1_fb)
    dW2 = np.dot(a1.T, du2_fb)
    
    W0 += 0.02 * dW0
    W1 += 0.02 * dW1
    W2 += 0.02 * dW2

    W0 = norm(W0)
    W1 = norm(W1)
    W2 = norm(W2)

    metrics[epoch] = (np.linalg.norm(du2_fb), )
    if epoch % 1 == 0:
        print("{} {:.4f} {:.4f} {:.4f} {:.4f}".format(
            epoch,
            np.linalg.norm(du2_fb),
            np.linalg.norm(relu_rc(x, W0) - a0_fb),
            np.linalg.norm(relu_rc(a0_fb, W1) - a1_fb),
            np.linalg.norm(relu_rc(a1_fb, W2) - a2_fb),
        ))

# shl(metrics)
# W1c = W1.copy()
# W1[np.where(np.random.random(W1.shape) < 0.05)] = 0.5
# shm(f(np.dot(np.dot(a0, W1), W1.T)), a0)