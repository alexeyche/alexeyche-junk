
import numpy as np
from poc.datasets import one_hot_encode
from poc.util import *

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


class Relu(Act):
    def __call__(self, x):
        return np.maximum(x, 0.0)
        
    def deriv(self, x):
        if isinstance(x, float):
            return 1.0 if x > 0.0 else 0.0
        dadx = np.zeros(x.shape)
        dadx[np.where(x > 0.0)] = 1.0
        return dadx

class ReluBound(Act):
    def __call__(self, x):
        return np.minimum(np.maximum(x, 0.0), 5.0)
        
    def deriv(self, x):
        if isinstance(x, float):
            return 1.0 if x > 0.0 else 0.0
        dadx = np.zeros(x.shape)
        dadx[np.where(x > 0.0)] = 1.0
        return dadx