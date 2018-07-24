

from poc.util import *
import numpy as np


threshold_value = 0.0
def threshold(x):
    a = np.zeros(x.shape)
    a[np.where(x >= threshold_value)] = 1.0
    return a

linear = lambda x: x
relu = lambda x: np.maximum(x, 0.0)

f = threshold

# f = linear
# f = relu

input_size = 10
batch_size = 5

layer_size = 100

W = np.random.random((input_size, layer_size),) - 0.5

x = f(np.random.random((batch_size, input_size))-0.5)

a_t = f(np.random.random((batch_size, layer_size))-0.5)


for epoch in range(1000):
    a = f(np.dot(x, W))
    x_t = f(np.dot(a, W.T))

    du = a_t - a

    dW = np.dot(x.T, du)

    W += 0.1 * dW

    if epoch % 100 == 0:
        print("{} {:.4f}, ta {:.4f}".format(epoch, np.linalg.norm(du), np.linalg.norm(x_t - x)))

