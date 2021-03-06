import numpy as np
from common_check import grad_check

class Activation(object):
    def __call__(self, x):
        raise NotImplementedError

    def grad(self, x):
        raise NotImplementedError    


    def approx_grad(self, x, epsilon=1e-05):
        dx = np.zeros(x.shape)
        for i in xrange(dx.shape[0]):
            for j in xrange(dx.shape[1]):
                de = np.zeros(dx.shape)
                de[i, j] = epsilon

                lo = self(x - de)
                ro = self(x + de)

                dx[i, j] = np.sum((ro - lo)/(2.0*epsilon))
        return dx


class ClipActivation(Activation):
    def __call__(self, x):
        return np.clip(x, 0.0, 1.0)
        # return np.clip(x + 0.1*np.random.randn(*x.shape), 0.0, 1.0)

    def grad(self, x):
        dx = np.ones(x.shape)
        dx[np.where(x < 0.0)] = 0.0
        dx[np.where(x > 1.0)] = 0.0
        return dx


class ExpClipActivation(Activation):
    def __call__(self, x):
        return np.exp(np.clip(x, 0.0, 1.0))

    def grad(self, x):
        dx = np.exp(x)
        dx[np.where(x < 0.0)] = 0.0
        dx[np.where(x > 1.0)] = 0.0
        return dx



class ExpActivation(Activation):
    def __call__(self, x):
        return np.exp(x)

    def grad(self, x):
        return np.exp(x)


class SoftplusActivation(Activation):
    def __call__(self, x):
        return np.log(1.0 + np.exp(x-1.0))

    def grad(self, x):
        return 1.0/(1.0 + np.exp(-(x-1.0)))


class SigmoidActivation(Activation):
    def __call__(self, x):
        v = 1.0/(1.0 + np.exp(-x))
        # return np.floor(np.random.random(v.shape) + v)
        return v

    def grad(self, x):
        v = 1.0/(1.0 + np.exp(-x))
        return v * (1.0 - v)


def test_act_grad(act, x, epsilon=1e-05, tol=1e-05, fail=True):
    
    dy = act.grad(x)
    
    dy_approx = act.approx_grad(x, epsilon=epsilon)
    
    grad_check(dy, dy_approx, "act", fail, tol)

    return dy, dy_approx



if __name__ == '__main__':
    test_act_grad(
        ExpClipActivation(), 
        np.random.random((10, 10))
    )