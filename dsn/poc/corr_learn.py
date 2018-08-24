

import numpy as np
from poc.common import *
from poc.util import *
from sklearn.neural_network._stochastic_optimizers import *

np.random.seed(12)

x = (np.random.random((100, 20)) < 0.1).astype(np.float32)
y = (np.random.random((100, 10)) < 0.1).astype(np.float32)

input_size = x.shape[1]
layer_size = 1000
output_size = y.shape[1]
batch_size = x.shape[0]

m_lam = 0.99

# f = relu
threshold_value = 0.4
f = lambda x: threshold(x, threshold_value)
fprime = lambda x: threshold_prime(x, threshold_value)

# f = lambda x: threshold_p(x, 0.1)

W0 = np.random.random((input_size, layer_size)) - 0.5
R0 = np.random.random((layer_size, input_size)) - 0.5

W1 = np.random.random((layer_size, output_size)) - 0.5
R1 = np.random.random((output_size, layer_size)) - 0.5


a0_hat = (np.random.random((batch_size, layer_size)) < 0.1).astype(np.float32)

params = (W0, W1, R1)
# opt = SGDOptimizer(params, learning_rate_init=0.01)
opt = AdamOptimizer(params, learning_rate_init=0.01)

for epoch in range(1000):
    u0 = np.dot(x, W0)
    a0 = f(u0)

    u1 = np.dot(a0, W1)
    a1 = f(u1)

    a1_fb = y
    a0_fb = f(np.dot(a1_fb, R1))


    du1 = a1_fb - a1

    # du0 = a0_fb - a0

    du0 = np.dot(du1, W1.T)

    dW1 = np.dot(a0.T, du1 * fprime(u1))
    dW0 = np.dot(x.T, du0 * fprime(u0))

    dR1 = np.dot(a1_fb.T, a0_fb - a0)

    opt.update_params((-dW0, -dW1, -0.0*dR1,))
    
    if epoch % 100 == 0:
        print("{} |{:.4f}| %{:.4f}, |{:.4f}| %{:.4f}, |dW0| = {:.4f}, |dW1| = {:.4f}, |dR1| = {:.4f}".format(
            epoch, 
            np.linalg.norm(a0 - a0_fb), number_of_equal_act(a0, a0_fb), 
            np.linalg.norm(a1 - a1_fb), number_of_equal_act(a1, a1_fb), 
            np.linalg.norm(dW0),
            np.linalg.norm(dW1),
            np.linalg.norm(dR1),
        ))


