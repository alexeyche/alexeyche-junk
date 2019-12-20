
import numpy as np
from collections import defaultdict

from metrics import *

def list_hash(d):
    assert d is not None and len(d) > 0

    value = d[:, 0] << 7
    for col_id in range(d.shape[1]):
        value = (101 * value + d[:, col_id]) & 0xfffffff
    return value


class LSH(object):
    def __init__(self, dim, hash_length, metric, bin_size=1.0, binary=False):
        self.hash_length = hash_length
        self.bin_size = bin_size
        self.dim = dim
        self.W = np.random.randn(self.dim, self.hash_length)
        self.buckets = defaultdict(list)
        self.metric = metric
        self.binary = binary

    def train(self, x):
        assert self.dim == x.shape[1]
        bins = self.get_bins(x)
        h = list_hash(bins)
        for ri in range(x.shape[0]):
            self.buckets[h[ri]].append(ri)
        self._x_train = x

    def get_bins(self, x):
        if self.binary:
            return ((np.sign(np.dot(x, self.W)) + 1.0) / 2.0).astype(np.bool)
        return np.floor(np.dot(x, self.W) / self.bin_size).astype(np.int32)

    def query(self, xt, nn):
        assert self.dim == xt.shape[1]
        assert len(self.buckets) > 0, "not trained"

        bins = self.get_bins(xt)
        h = list_hash(bins)

        ans = -np.ones((xt.shape[0], nn), dtype=np.int32)
        distances = -np.ones((xt.shape[0], nn), dtype=np.float32)

        for ri in range(xt.shape[0]):
            if h[ri] in self.buckets:
                elements = np.asarray(self.buckets[h[ri]])
                d_elements = np.asarray([
                    self.metric(self._x_train[train_id], xt[ri])
                    for train_id in elements
                ])

                if len(elements) <= nn:
                    s_ids = np.argsort(d_elements)
                else:
                    s_ids = np.argpartition(d_elements, nn)[:nn]
                    s_ids = np.argsort(d_elements[s_ids])

                distances[ri, :len(s_ids)] = d_elements[s_ids]
                ans[ri, :len(s_ids)] = elements[s_ids]

        return distances, ans
