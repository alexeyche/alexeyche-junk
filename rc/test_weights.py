
from util import *
from config import *

def synaptic_normalization(W):
    z = np.sum(np.abs(W), axis=0)
    z[z < 1e-6] = 1e-6
    W /= z[None, :]

x0 = np.zeros((1, 10,))
x1 = np.zeros((1, 10,))

y0 = np.zeros((1, 1,))
y1 = np.zeros((1, 1,))

W = np.ones((10, 1))

synaptic_normalization(W)


def stdp(x0, x1, y0=None, y1=None):
    assert (y0 is not None) == (y1 is not None), \
        "y0 y1 should be both not none"

    if y0 is None and y1 is None:
        A = np.dot(x0.T, x1)
        return A - A.T

    return np.dot(x0.T, y1) - np.dot(x1.T, y0)

    # dW = np.zeros((x0.shape[-1], y0.shape[-1]))

    # for i in range(dW.shape[0]):
    #     for j in range(dW.shape[1]):
    #         dW[i, j] = x0[i] * y1[j]  - x1[i] * y0[j]

    # return x0[:, None] * y1[None, :] - x1[:, None] * y0[None,:]


for _ in range(10):
    x0[0, 0] = 1.0
    x0[0, 1] = 1.0

    x1[0, 0] = 1.0
    x1[0, 1] = 1.0

    y0[0, 0] = 1.0
    y1[0, 0] = 0.0


    # x0[1] = 1.0
    # x1[2] = 1.0


    # A = np.dot(x.T, x_new)

    # A = np.outer(x, x_new)

    # dW = np.zeros(W.shape)

    # for i in range(W.shape[0]):
    #     for j in range(W.shape[1]):
    #         dW[i, j] = x_new[i] * x[j] - x[i] * x_new[j]

    # dW = x_new[:,None] * x[None,:] - x[:,None] * x_new[None,:]

    dW = stdp(x0, x1, y0, y1)

    # dW = A - A.T

    # W += 0.01 * dW

    # W = np.where(W < 0, 0.0, W)
    # W = np.where(W > 1.0, 1.0, W)


