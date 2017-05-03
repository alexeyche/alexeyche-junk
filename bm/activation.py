import numpy as np

class Activation(object):
    def __call__(self, x):
        raise NotImplementedError

    def grad(self, x):
        raise NotImplementedError    


class ClipActivation(Activation):
    def __call__(self, x):
        return np.clip(x, 0.0, 1.0)

    def grad(self, x):
        dx = np.ones(x.shape)
        dx[np.where(x < 0.0)] = 0.0
        dx[np.where(x > 1.0)] = 0.0
        return dx



class ExpActivation(Activation):
    def __call__(self, x):
        return np.clip(np.exp(x)-1.0, 0.0, 1.0)

    def grad(self, x):
        dx = np.exp(x)
        dx[np.where(x < 0.0)] = 0.0
        dx[np.where(x > 1.0)] = 0.0
        return dx


class SigmoidActivation(Activation):
    def __call__(self, x):
        v = 1.0/(1.0 + np.exp(-(x-2.0)))
        return np.floor(np.random.random(v.shape) + v)

    def grad(self, x):
        v = 1.0/(1.0 + np.exp(-(x-2.0)))
        return v * (1.0 - v)
