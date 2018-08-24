
import numpy as np
from poc.common import *
from poc.util import *

np.random.seed(10)

x = (np.random.random((5, 10)) < 0.1).astype(np.float32)
y = (np.random.random((5, 10)) < 0.1).astype(np.float32)



input_size = x.shape[1]
layer_size = 200
output_size = y.shape[1]
batch_size = x.shape[0]
num_iters = 1000
threshold_value = 0.1

W = np.random.random((layer_size, layer_size),) - 0.5
W = norm(W)


u = np.zeros((batch_size, layer_size))
a = np.zeros((batch_size, layer_size))
ah = np.zeros((num_iters, batch_size, layer_size))
uh = np.zeros((num_iters, batch_size, layer_size))
dWh = np.zeros((num_iters, layer_size, layer_size))

dW = np.zeros((layer_size, layer_size))

for iter in range(num_iters):
    a[:, :input_size] = x.copy()
    a[:, -output_size:] = y.copy()

    u += 0.1 * (np.dot(a, W) - u)

    new_a = threshold(u, threshold_value)

    ah[iter] = new_a.copy()
    uh[iter] = u.copy()

    dW = np.dot(a.T, new_a) / layer_size - 0.1 * W
    dWh[iter] = dW.copy()

    a = new_a

    # W += 0.01 * dW

    print("{} {:.4f}".format(iter, np.linalg.norm(dW)))
