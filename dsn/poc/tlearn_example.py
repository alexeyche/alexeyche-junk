

from poc.util import *
import numpy as np
from poc.common import *




input_size = 100
batch_size = 100
num_iters = 100
output_size = 100

average = 10
k = 0.1
# f = lambda x: threshold_k(x, k)
# f = linear
f = lambda x: threshold_k(x, 1)


layer_size = 1000
p = 0.9


x = np.random.random((num_iters, batch_size, input_size,)).astype(np.float32)
y = np.random.random((num_iters, batch_size, output_size,)).astype(np.float32)

W = random_sparse((input_size, layer_size), p=0.9)

m = 0.0
for ti in range(num_iters):
    xt = f(x[ti])

    at = f(np.dot(xt, W))

    xt_tr = f(np.dot(at, W.T))


    m += number_of_equal_act(xt, xt_tr) / num_iters

print(m)


