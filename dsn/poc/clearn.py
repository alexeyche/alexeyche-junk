
import numpy as np
from poc.common import *
from poc.util import *

np.random.seed(12)

x = (np.random.random((10, 20)) < 0.1).astype(np.float32)
y = (np.random.random((10, 10)) < 0.1).astype(np.float32)

input_size = x.shape[1]
layer_size = 100
output_size = y.shape[1]
batch_size = x.shape[0]


a0_t = (np.random.random((batch_size, layer_size)) < 0.1).astype(np.float32)


f = lambda x: threshold(x, 0.25)

W0 = np.random.random((input_size, layer_size)) - 0.5
R0 = np.random.random((layer_size, input_size)) - 0.5

W0 = norm(W0)
R0 = norm(R0)

for epoch in range(5500):
    a0 = f(np.dot(x, W0))
    x_fb = f(np.dot(a0, R0))

    # dW0 = np.dot((x-np.dot(a0, W0.T)).T, a0)
    # dR0 = np.dot((a0-np.dot(x-x_fb, R0.T)).T, x-x_fb)

    dW0 = np.dot(x.T, a0_t-a0) / batch_size
    dR0 = np.dot(a0_t.T, x-x_fb) / batch_size

    W0 += 0.01 * dW0
    R0 += 0.01 * dR0

    # W0 = norm(W0)
    # R0 = norm(R0)

    if epoch % 100 == 0:
        print("{} {:.4f} {:.4f} {:.4f} {:.4f}, |dW0| = {:.4f}, |dR0| = {:.4f}".format(
            epoch,
            np.linalg.norm(x - x_fb), number_of_equal_act(x_fb, x),
            np.linalg.norm(a0_t - a0), number_of_equal_act(a0, a0_t),
            np.linalg.norm(dW0),
            np.linalg.norm(dR0),
        ))