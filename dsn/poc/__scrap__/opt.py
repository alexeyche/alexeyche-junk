import numpy as np
from scipy.sparse import csr_matrix


class Optimization(object):
    def init(self, *shapes):
        raise NotImplementedError

    def update_params(self, *dparams):
        raise NotImplementedError


class MomentumOpt(Optimization):
    def __init__(self, learning_rates, gamma):
        self.gamma = gamma
        # self.learning_rates = [ lr * (1.0 - gamma) for lr in learning_rates]
        self.learning_rates = learning_rates

        self.moments = []
        self.params = []

    def init(self, *params):
        self.moments = []

        for param in params:
            self.moments.append(np.zeros(param.shape))
        self.params = list(params)

    def update(self, *dparams):
        assert len(self.moments) == len(dparams)
        assert len(self.params) == len(dparams)

        for pi, dparam in enumerate(dparams):
            self.moments[pi] = self.moments[pi] * self.gamma + self.learning_rates[pi] * dparam
            self.params[pi] -= self.moments[pi]


class SGDOpt(Optimization):
    def __init__(self, learning_rates):
        self.learning_rates = learning_rates
        self.params = []

    def init(self, *params):
        self.params = list(params)

    def update(self, *dparams):
        for pi, dparam in enumerate(dparams):
            self.params[pi] -= self.learning_rates[pi] * dparam


class AdagradOpt(Optimization):
    def __init__(self, learning_rates, eps=1e-05):
        self.learning_rates = learning_rates
        self.eps = eps
        self.cache = []
        self.params = list()

    def init(self, *params):
        self.cache = [np.zeros(p.shape) for p in params]
        self.params = list(params)

    def update(self, *dparams):
        assert len(self.cache) == len(dparams)

        for pi, dparam in enumerate(dparams):
            self.cache[pi] += np.square(dparam)
            self.params[pi] -= self.learning_rates[pi] * dparam / (np.sqrt(self.cache[pi]) + self.eps)


class AdadeltaOpt(Optimization):
    """
    TODO: it's not Adadelta actually
    """

    def __init__(self, learning_rates, gamma, eps=1e-05):
        self.learning_rates = learning_rates
        self.eps = eps
        self.cache = []
        self.params = list()
        self.gamma = gamma

    def init(self, *params):
        self.cache = [np.zeros(p.shape) for p in params]
        self.params = list(params)

    def update(self, *dparams):
        assert len(self.cache) == len(dparams)

        for pi, dparam in enumerate(dparams):
            self.cache[pi] = self.gamma * self.cache[pi] + (1.0 - self.gamma) * np.square(dparam)
            self.params[pi] -= self.learning_rates[pi] * dparam / (np.sqrt(self.cache[pi]) + self.eps)


class RMSPropOpt(Optimization):
    def __init__(self, learning_rates, decay_rate=0.9, eps=1e-05):
        self.learning_rates = learning_rates
        self.eps = eps
        self.decay_rate = decay_rate
        self.cache = []
        self.params = []

    def init(self, *params):
        self.cache = [np.zeros(p.shape) for p in params]
        self.params = list(params)

    def update(self, *dparams):
        assert len(self.cache) == len(dparams)

        for pi, dparam in enumerate(dparams):
            self.cache[pi] = self.cache[pi] * self.decay_rate + (1.0 - self.decay_rate) * np.square(dparam)

            self.params[pi] -= self.learning_rates[pi] * dparam / (np.sqrt(self.cache[pi]) + self.eps)


class AdamOpt(Optimization):
    def __init__(self, params, learning_rate, beta1=0.9, beta2=0.999, eps=1e-08):
        self.learning_rate = learning_rate

        self.eps = eps

        self.beta1 = beta1
        self.beta2 = beta2

        self.m = [np.zeros(p.shape) for p in params]
        self.v = [np.zeros(p.shape) for p in params]
        self.params = [param for param in params]

    def update_params(self, dparams):
        assert len(self.m) == len(dparams) and len(self.v) == len(dparams)

        for pi, dparam in enumerate(dparams):
            self.m[pi] = self.m[pi] * self.beta1 + (1.0 - self.beta1) * dparam
            self.v[pi] = self.v[pi] * self.beta2 + (1.0 - self.beta2) * np.square(dparam)

            self.params[pi] -= self.learning_rate * self.m[pi] / (np.sqrt(self.v[pi]) + self.eps)




class AdamOptSparse(Optimization):
    def __init__(self, params, learning_rate, beta1=0.9, beta2=0.999, eps=1e-08):
        self.learning_rate = learning_rate

        self.eps = eps

        self.beta1 = beta1
        self.beta2 = beta2

        self.m = [np.zeros(p.shape) for p in params]
        self.v = [np.zeros(p.shape) for p in params]
        self.params = [param for param in params]

    def update_params(self, dparams):
        assert len(self.m) == len(dparams) and len(self.v) == len(dparams)

        for pi, dparam in enumerate(dparams):
            self.m[pi] = self.m[pi] * self.beta1 + (1.0 - self.beta1) * dparam.todense()
            self.v[pi] = self.v[pi] * self.beta2 + (1.0 - self.beta2) * np.square(dparam.todense())

            self.params[pi] -= csr_matrix(self.learning_rate * self.m[pi] / (np.sqrt(self.v[pi]) + self.eps))












