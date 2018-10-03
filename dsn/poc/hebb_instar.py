
import numpy as np
from poc.common import *
from poc.util import *


# If 0 in input, you can get an error in MSE (duh)

def unit_vector(data, axis=0):
    data_denom = np.sqrt(np.sum(data ** 2, keepdims=True, axis=axis))
    non_null = np.where(data_denom > 1e-10)
    data[non_null] = data[non_null]/data_denom[non_null]
    return data

def angle(x, y):
    return np.mean(np.arccos(
        np.clip(
            np.dot(unit_vector(x), unit_vector(y)),
            -1.0,
            1.0
        )
    ))

# np.random.seed(12)

x = (np.random.random((5, 10)) < 0.1).astype(np.float32)
y = (np.random.random((5, 10)) < 0.1).astype(np.float32)



input_size = x.shape[1]
layer_size = 50
output_size = y.shape[1]
batch_size = x.shape[0]
num_iters = 1000
threshold_value = 0.1
K = 1  # 2 also seem to work

W = np.random.random((input_size, layer_size),) - 0.5
W = norm(W)

Wo = np.zeros((layer_size, output_size),)


u = np.zeros((batch_size, layer_size))
a = np.zeros((batch_size, layer_size))
ah = np.zeros((num_iters, batch_size, layer_size))
uh = np.zeros((num_iters, batch_size, layer_size))
Wh = np.zeros((num_iters, input_size, layer_size))
angle_h = np.zeros((num_iters, 2))

for iter in range(num_iters):
    u = np.dot(x, W)
    a = threshold_k(u, K)

    uo = np.dot(a, Wo)
    ao = threshold(uo, threshold_value)


    ah[iter] = a.copy()
    uh[iter] = u.copy()

    angle_l = 0.0
    dW = np.zeros(W.shape)
    for ni in range(layer_size):
        angle_l += angle(x, W[:,ni]) / layer_size
        dW[:, ni] = np.dot(a[:, ni].T, x - W[:, ni])

    angle_o = 0.0
    dWo = np.zeros(Wo.shape)

    for oi in range(output_size):
        angle_o += angle(a, Wo[:, oi]) / output_size

        for ni in range(layer_size):
            dWo[ni, oi] = np.sum(a[:, ni] * (y[:, oi] - Wo[ni, oi]))

    W += 0.01 * dW
    Wo += 0.001 * dWo

    Wh[iter] = W.copy()
    angle_h[iter, 0] = angle_l
    angle_h[iter, 1] = angle_o

    print("{} |dW| {:.4f} |dWo| {:.4f} angles {:.4f} {:.4f} MSE {:.4f}".format(
        iter, 
        np.linalg.norm(dW), 
        np.linalg.norm(dWo), 
        angle_l, 
        angle_o,
        np.linalg.norm(y - ao)
    ))


# shl(angle_h)