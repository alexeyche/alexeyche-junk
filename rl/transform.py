
import numpy as np
import logging

from sklearn.preprocessing import StandardScaler

log = logging.getLogger(__name__)



class Transform(object):
    def __init__(self, filter, callback, is_semigroup=False):
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
            log.warn("Got empty data after filter {}".format(self.filter.__name__))
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




def z_score(x):
    std = StandardScaler()
    return std.fit_transform(x)

def sum(*args):
    a0 = args[0].copy()
    for a1 in args[1:]:
        a0 += a1
    return a0


def identity(x):
    return x

TIdentity = Transform(Filters.all(), identity)


TSin = Transform(Filters.all(), np.sin)
TCos = Transform(Filters.all(), np.cos)
TZScore = Transform(Filters.all(), z_score)
TExp = Transform(Filters.less_than(10.0), np.exp)
TSqrt = Transform(Filters.positive(), np.sqrt)
TSquare = Transform(Filters.all(), np.square)
TLog = Transform(Filters.positive(), np.log)
TTanh = Transform(Filters.all(), np.tanh)


TSum = Transform(Filters.all(), sum, is_semigroup=True)





