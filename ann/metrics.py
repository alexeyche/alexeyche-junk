
import numpy as np

from scipy.spatial.distance import pdist as scipy_pdist
from sklearn.neighbors import NearestNeighbors as sknn

from util import Timing

def knn(dataset_distances, run_distances, count, epsilon=1e-10):
    total = len(run_distances) * count
    actual = 0
    for true_distances, found_distances in zip(dataset_distances, run_distances):
        within = [
            d
            for d in found_distances[:count]
            if d <= true_distances[count - 1] + epsilon
        ]
        actual += len(within)
    return float(actual) / float(total)

def pdist(a, b, metric):
    return scipy_pdist([a, b], metric=metric)[0]

def angular(a, b):
    return pdist(a, b, "cosine")

def euclidean(a, b):
    return pdist(a, b, "euclidean")


def get_knn(x, xt, nn, verbose=False):
    with Timing("KNN brute", verbose):
        bmodel = sknn(algorithm="brute", metric="euclidean")
        bmodel.fit(x)
        d, idx = bmodel.kneighbors(xt, return_distance=True, n_neighbors=nn)
    return d, idx


def calc_ids(x, xt, nn, cb, cb_t=None, metric="euclidean"):
    ids = cb(x)
    test_ids = cb(xt) if cb_t is None else cb_t(xt)
    _, n_ids = get_knn(ids, test_ids, nn)
    return n_ids, ids, test_ids



def nn_precision(idx_baseline, idx_next):
    assert idx_baseline.shape == idx_next.shape

    nn = idx_baseline.shape[1]
    return (
        np.mean(
            list(
                map(lambda x: len(x) / float(nn),
                    map(np.intersect1d, idx_baseline, idx_next))
            )
        )
    )


