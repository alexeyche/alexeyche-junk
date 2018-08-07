

from poc.util import *
import numpy as np
from poc.common import number_of_equal_act

def threshold_k(u, pk):
    K = int(u.shape[1] * pk)
    # K = 1
    a = np.zeros(u.shape)
    batch_size = u.shape[0]

    bidx = np.arange(0, batch_size)
    ind = np.argpartition(u, -K, axis=1)[:, -K:]

    a[np.expand_dims(bidx, 1), ind] = 1.0
    a[np.where(np.abs(u) < 1e-10)] = 0.0
    return a


# relu = lambda x: np.maximum(x, 0.0)



input_size = 100
batch_size = 100


average = 10
k = 0.1

f = lambda x: threshold_k(x, k)
# f = linear
# f = relu


layer_sizes = range(10, 200, 10)
layer_size = 100
p = 0.9

# for li, layer_size in enumerate(layer_sizes):
m = 0.0

for _ in range(average):
    # W = np.random.random((input_size, layer_size),)

    # W = random_orth((input_size, layer_size,), )
    # W = random_orth((layer_size, input_size, ), ).T

    # W = random_pos_sparse((input_size, layer_size), p=p)

    # W = random_pos_orth((input_size, layer_size), )
    W = random_pos_orth((layer_size, input_size), ).T

    a = f(np.random.random((batch_size, layer_size)))

    x = f(np.dot(a, W.T))
    a_t = f(np.dot(x, W))


    m += number_of_equal_act(a, a_t) / float(average)

print(layer_size, p, m, np.linalg.norm(np.dot(W,W.T) - np.eye(input_size)))

# shm(a.T, a_t.T, a_t.T-a.T)