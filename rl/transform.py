
import numpy as np
import pandas as pd
import logging

from sklearn.preprocessing import StandardScaler
from sklearn.preprocessing import MinMaxScaler

log = logging.getLogger(__name__)



class Transform(object):
    ALL_TRANSFORMS = []

    def __init__(self, filter, callback, is_semigroup=False, register=True):
        if register:
            Transform.ALL_TRANSFORMS.append(self)

        self.filter = filter
        self.callback = callback
        self.is_semigroup = is_semigroup

    def __call__(self, *xs):
        assert len(xs) == 1 or self.is_semigroup, \
            "Expecting one argument for non semigroup transform"
        log.debug(
            "Apply T={} and F={}, x={}".format(
                self.name,
                self.filter.__name__,
                ",".join([str(x.shape) for x in xs])
            )
        )
        xf = [self.filter(x) for x in xs]
        xf = [x for x in xf if x.shape[1] > 0]

        if len(xf) == 0:
            log.debug("Got empty data after filter {}".format(self.filter.__name__))
            return
        return self.callback(*xf)

    @property
    def name(self):
        return self.callback.__name__

    @property
    def __name__(self):
        return self.name

    def __hash__(self):
        return hash(self.name)

    def __eq__(self, x):
        return self.name == x.name

    def __repr__(self):
        return self.name


class Filters(object):
    @staticmethod
    def all():
        def no_filter_cb(x):
            return x
        return no_filter_cb

    @staticmethod
    def less_than(upper):
        def less_than_cb(x):
            d = x.describe()
            return x[d.columns[d.loc["max"].lt(upper).values]]
        return less_than_cb

    @staticmethod
    def positive():
        def positive_cb(x):
            d = x.describe()
            return x[d.columns[d.loc["min"].gt(0.0).values]]
        return positive_cb

    @staticmethod
    def non_zeros(lower_ratio):
        epsilon = 1e-08

        def non_zeros_cb(x):
            stat = (
                x.apply(
                    lambda c: 1.0 - np.sum(c.apply(lambda v: np.abs(v) < epsilon)) / c.shape[0],
                    axis=0
                )
            )
            return x[x.columns[stat.gt(lower_ratio - epsilon).values]]
        return non_zeros_cb



def std_norm(x):
    scaler = StandardScaler()
    return pd.DataFrame(scaler.fit_transform(x.values), columns=x.columns)

def min_max_norm(x):
    scaler = MinMaxScaler()
    return pd.DataFrame(scaler.fit_transform(x.values), columns=x.columns)

def semigroup_op(op, *args):
    a0 = args[0].copy()
    for a1 in args[1:]:
        c_common = a0.columns & a1.columns
        a0 = a0[c_common]
        if len(c_common) == 0:
            pass
        else:
            a0[c_common] = op(a0[c_common], a1[c_common])
    return a0

def sum(*args):
    return semigroup_op(lambda a, b: a + b, *args)

def product(*args):
    return semigroup_op(lambda a, b: a * b, *args)

def division(*args):
    return semigroup_op(lambda a, b: a / b, *args)

def identity(x):
    return x

def sigmoid(x):
    return 1.0 / (1.0 + np.exp(-x))

TIdentity = Transform(Filters.all(), identity, register=False)


TLog = Transform(Filters.positive(), np.log)
TSquare = Transform(Filters.all(), np.square)
TSqrt = Transform(Filters.positive(), np.sqrt)
TProduct = Transform(Filters.all(), sum, is_semigroup=True)
TStdNorm = Transform(Filters.all(), std_norm)
TMinMaxNorm = Transform(Filters.all(), min_max_norm)
TSum = Transform(Filters.all(), sum, is_semigroup=True)
TDivision = Transform(Filters.non_zeros(lower_ratio=1.0), division, is_semigroup=True)
TSigmoid = Transform(Filters.all(), sigmoid)
TSin = Transform(Filters.all(), np.sin)
TCos = Transform(Filters.all(), np.cos)
TExp = Transform(Filters.less_than(10.0), np.exp)
TTanh = Transform(Filters.all(), np.tanh)








