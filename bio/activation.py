import numpy as np
from common_check import grad_check, check_matrix

class Activation(object):
    def __call__(self, x):
        raise NotImplementedError

    def grad(self, x):
        raise NotImplementedError    


    def approx_grad(self, x, epsilon=1e-05):
        return check_matrix(x, self, epsilon=epsilon)


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
    return grad_check(
        act.grad(x), 
        act.approx_grad(x, epsilon=epsilon), 
        "act", 
        fail, 
        tol
    )


if __name__ == '__main__':
    test_act_grad(
        SoftplusActivation(), 
        np.random.random((10, 10))
    )