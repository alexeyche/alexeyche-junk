
import numpy as np
from poc.common import *
from poc.util import *
from sklearn.neural_network._stochastic_optimizers import *

# dW1 = np.dot(a0.T, a1_fb - a1) / batch_size
# dW0 = np.dot(x.T, a0_fb - a0) / batch_size
#
# dR1 = np.dot(a1_fb.T, a0 - a0_fb) / batch_size
# dR0 = np.dot(a0_fb.T, x - x_fb) / batch_size

# np.random.seed(12)

x = (np.random.random((100, 20)) < 0.1).astype(np.float32)
y = (np.random.random((100, 50)) < 0.1).astype(np.float32)

input_size = x.shape[1]
layer_size = 200
output_size = y.shape[1]
batch_size = x.shape[0]

m_lam = 0.99

# f = relu
f = lambda x: threshold(x, 0.5)
# f = lambda x: threshold_p(x, 0.1)

sd = 0.5
W0 = sd * (np.random.random((input_size, layer_size)) - 0.5)
R0 = sd * (np.random.random((layer_size, input_size)) - 0.5)

W1 = sd * (np.random.random((layer_size, output_size)) - 0.5)
R1 = sd * (np.random.random((output_size, layer_size)) - 0.5)


params = (W0, R0, W1, R1)
opt = SGDOptimizer(params, learning_rate_init=0.0001)
# opt = AdamOptimizer(params, learning_rate_init=0.005)

a0m = np.zeros((layer_size,))

for epoch in range(3000):
    a0 = f(np.dot(x, W0))
    a1 = f(np.dot(a0, W1))

    # a1_fb = y
    a1_fb = a1

    a0_fb = f(np.dot(a1_fb, R1))
    x_fb = f(np.dot(a0_fb, R0))


    a0m = m_lam * a0m + (1.0-m_lam) * np.mean(a0, 0)

    dW0 = np.dot((x-x_fb).T, a0)
    dR0 = np.dot(a0.T, x - x_fb)

    dW1 = np.dot((a0-a0_fb).T, a1)
    dR1 = np.dot(a1.T, a0-a0_fb)

    # dW1 = np.dot(a0.T, a1_fb - a1)
    # dW0 = np.dot(x.T, a0_fb - a0)
    #
    # dR1 = np.dot(a1_fb.T, a0 - a0_fb)
    # dR0 = np.dot(a0_fb.T, x - x_fb)

    opt.update_params((-dW0, -dR0, -dW1, -dR1))

    if epoch % 100 == 0:
        print("{} {:.4f} {:.4f} & {:.4f} {:.4f}, |dW0| = {:.4f}, |dR0| = {:.4f}, |dW1| = {:.4f}, |dR1| = {:.4f}".format(
            epoch,
            np.linalg.norm(x - x_fb), number_of_equal_act(x_fb, x),
            np.linalg.norm(a0_fb - a0), number_of_equal_act(a0, a0_fb),
            np.linalg.norm(dW0),
            np.linalg.norm(dR0),
            np.linalg.norm(dW1),
            np.linalg.norm(dR1),
        ))