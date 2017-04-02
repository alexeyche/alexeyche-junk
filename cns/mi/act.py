
import numpy as np

class Act(object):
    def __call__(self, x):
        raise NotImplementedError()

    def deriv(self, x):
        raise NotImplementedError()

class Linear(Act):
    def __call__(self, x):
        deriv = 1.0
        if hasattr(x, "shape"):
            deriv = np.ones(x.shape)

        return x, deriv


class Sigmoid(Act):
    def __call__(self, x):
        res = 1.0/(1.0 + np.exp(-x))
        return res, res * (1.0 - res) 


class Tanh(Act):
    def __call__(self, x):
        e = np.exp(2.0 * x)
        res = (e - 1.0)/(e + 1.0)

        return res, 1.0 - res * res


class Relu(Act):
    def __call__(self, x):
        x_non_z = np.where(x > 0)
        res = np.zeros(x.shape)
        res[x_non_z] = x[x_non_z]
        dres = np.zeros(x.shape)
        dres[x_non_z] = 1.0
        return res, dres



def get_oja_deriv(x, y, W, dy):
    assert W.shape[0] == len(x), "x, {} != {}".format(W.shape[0], len(x))
    assert W.shape[1] == len(y), "y, {} != {}".format(W.shape[1], len(y))

    dW = np.zeros(W.shape)
    for ni in xrange(len(y)):
        dW[:, ni] = y[ni] * (x - y[ni] * W[:, ni]) * dy[ni]
    return dW

