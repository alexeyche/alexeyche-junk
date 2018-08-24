
import numpy as np
from poc.datasets import *
from poc.util import *

from poc.common import *


# np.random.seed(11)
#
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

f, fprime = lambda x: threshold(x, 0.5), lambda x: threshold_prime(x, 0.5)
# f, fprime = threshold_k, threshold_prime

# f, fprime = linear, linear_prime

# np.random.seed(10)

# x = f(np.random.random((100, 20)))
# xl = np.dot(x, np.random.random((20, 10))-0.5)
# y = f(xl)

x = (np.random.random((100, 20)) < 0.1).astype(np.float32)
y = (np.random.random((100, 10)) < 0.1).astype(np.float32)



input_size = x.shape[1]
layer_size = 20
output_size = y.shape[1]
batch_size = x.shape[0]


# W0 = random_orth((input_size, layer_size))
# W1 = random_orth((layer_size, layer_size))
# W2 = random_orth((layer_size, output_size))

# W0 = random_pos_sparse((input_size, layer_size), p=0.75)
# W1 = random_pos_sparse((layer_size, layer_size), p=0.75)
# W2 = random_pos_sparse((layer_size, output_size), p=0.75)

W0 = np.random.random((input_size, layer_size),) - 0.5
W1 = np.random.random((layer_size, layer_size),) - 0.5
W2 = np.random.random((layer_size, output_size),)- 0.5

noise = 0.0
epochs = 5000
metrics = np.zeros((epochs,1))
for epoch in range(epochs):
    u0 = np.dot(x, W0)
    a0 = f(u0)

    u1 = np.dot(a0, W1)
    a1 = f(u1)

    u2 = np.dot(a1, W2)
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


    dW0 = np.dot(x.T, du0)
    dW1 = np.dot(a0.T, du1)
    dW2 = np.dot(a1.T, du2)
    #
    W0 += 0.05 * dW0
    W1 += 0.05 * dW1
    W2 += 0.05 * dW2

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