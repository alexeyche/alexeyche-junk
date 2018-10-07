
import numpy as np
from poc.datasets import *
from poc.util import *
from sklearn.neural_network._stochastic_optimizers import *

from poc.common import *


np.random.seed(13)

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


f, fprime = lambda x: threshold(x, 0.1), lambda x: threshold_prime(x, 0.1)

x = (np.random.random((100, 20)) < 0.1).astype(np.float32)
y = (np.random.random((100, 10)) < 0.1).astype(np.float32)



input_size = x.shape[1]
layer_size = 100
output_size = y.shape[1]
batch_size = x.shape[0]


W0 = np.random.random((input_size, layer_size),) - 0.5
b0 = np.random.random((layer_size,),) - 0.5
W1 = np.random.random((layer_size, output_size),) - 0.5
b1 = np.random.random((output_size,),) - 0.5

W1fb = np.random.random((output_size, layer_size),) - 0.5

params = (W0, b0, W1, b1)
opt = AdamOptimizer(params, learning_rate_init=0.001, beta_1=0.99)

noise = 0.0
epochs = 15000
metrics = np.zeros((epochs,2))
for epoch in range(epochs):
    u0 = np.dot(x, W0) + b0
    a0 = f(u0)

    u1 = np.dot(a0, W1) + b1
    a1 = f(u1)

    e = y - a1
    
    a1_fb = y
    a0_fb = f(np.dot(a1_fb, W1fb))

    du1_fb = y - a1
    du0_fb = a0 * (a0_fb - 0.5) * fprime(u0)

    # BP
    # du0_fb, du1_fb = np.dot(du1, W1fb) * fprime(u0), e

    dW0 = np.dot(x.T, du0_fb)
    dW1 = np.dot(a0.T, du1_fb)
    
    db0 = np.sum(du0_fb, 0)
    db1 = np.sum(du1_fb, 0)

    opt.update_params((-dW0, -db0, -dW1, -db1,))

    metrics[epoch] = (np.linalg.norm(e), np.linalg.norm(du0_fb),)
    if epoch % 100 == 0:
        print("{} {:.4f} {:.4f} {:.4f}".format(
            epoch,
            np.linalg.norm(du0_fb),
            np.linalg.norm(e),
            number_of_equal_act(a1, y)
        ))

