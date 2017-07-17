
import numpy as np

class Act(object):
    def __call__(self, x):
        raise NotImplementedError()

    def deriv(self, x):
        raise NotImplementedError()

class Linear(Act):
    def __call__(self, x):
        return x

    def deriv(self, x):
        if hasattr(x, "shape"):
            return np.ones(x.shape)
        return 1.0

class Sigmoid(Act):
    def __call__(self, x):
        return 1.0/(1.0 + np.exp(-x))

    def deriv(self, x):
        v = self(x)
        return v * (1.0 - v)

def oja_rule(x, y, W, dy):
    assert W.shape[0] == len(x), "x, {} != {}".format(W.shape[0], len(x))
    assert W.shape[1] == len(y), "y, {} != {}".format(W.shape[1], len(y))

    dW = np.zeros(W.shape)
    for ni in xrange(len(y)):
        # dW[:, ni] = y[ni] * x
        dW[:, ni] = y[ni] * (x - y[ni] * W[:, ni]) * dy[ni]
    return dW


class Learning(object):
    BP = 0
    FA = 1
    HEBB = 2
    OJA = 3
    OJA_FEED = 4


def norm(f):
    return np.asarray([ f[ri, :] * n for ri, n in enumerate(np.sqrt(np.sum(f ** 2, 1)+1e-07)) ])
    # return np.asarray([ f[:, ci] / n for ci, n in enumerate(np.sqrt(np.sum(f ** 2, 0)+1e-07)) ]).T
    # return f

