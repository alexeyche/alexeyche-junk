
import numpy as np
from matplotlib import pyplot as plt
import scipy.sparse as sp

from dataset import get_dataset
from util import *
from metrics import *
from bio_hash import *

from sklearn.preprocessing import MinMaxScaler

np.random.seed(11)

def plot_dist(ids0, ids1, ids2):
    fig = plt.figure(figsize=(25, 10))
    ax = fig.add_subplot(111)

    w = 0.3

    f = np.asarray(ids0.sum(axis=0))
    f = MinMaxScaler().fit_transform(f.T).reshape(-1)
    ax.bar(np.arange(f.shape[0]) - w, f[np.argsort(f)[::-1]], width=w)

    f = np.asarray(ids1.sum(axis=0, keepdims=True))
    f = MinMaxScaler().fit_transform(f.T).reshape(-1)
    ax.bar(np.arange(f.shape[0]), f[np.argsort(f)[::-1]], width=w)

    f = np.asarray(ids2.sum(axis=0, keepdims=True))
    f = MinMaxScaler().fit_transform(f.T).reshape(-1)
    ax.bar(np.arange(f.shape[0]) + w, f[np.argsort(f)[::-1]], width=w)

    ax.legend(("ids0", "ids1", "ids2"))

    fig.show()

D = get_dataset("mnist-784-euclidean")

x = get_random(np.array(D["train"]), 1000)
xt = get_random(x, 100)

nn = 50



# _, idx0 = get_knn(x, xt, nn)

K = 2000

algo = BioHash(
    dim=x.shape[1],
    K=K,
    p=6.0 / K,
    top_k=128
)

algo_rec = BioRecHash(
    dim=x.shape[1],
    K=K,
    p=6.0 / K,
    p_rec=6.0 / K,
    top_k=32,
    scale=0.01
)

# ids2, dW, t0, r0 = algo_rec.get_bins_dyn(
#     x,
#     t=1.0,
#     dt=0.1,
#     num_iters=15,
#     lrate=0.001,
#     num_epochs=10
# )




base_n, _, _ = calc_ids(x, xt, nn, lambda x: x, metric="euclidean")

n0, ids0, test_ids0 = (
    calc_ids(x, xt, nn, lambda x: algo.get_bins_binary(x), metric="hamming")
)

n1, ids1, test_ids1 = (
    calc_ids(x, xt, nn, lambda x: get_top_active_indices(algo.get_bins(x, t=10.0), 128), metric="hamming")
)

print("binary: {:.4f}".format(nn_precision(base_n, n0)))
print("dot: {:.4f}".format(nn_precision(base_n, n1)))

n2, ids2, test_ids2 = (
    calc_ids(
        x, xt, nn,
        lambda x: get_top_active_indices(algo.get_bins_dyn(
            x,
            t=10.0,
            dt=0.1,
            num_iters=10,
            lrate=0.01,
            num_epochs=25
        ), 128),
        lambda xt: get_top_active_indices(algo.get_bins_dyn(
            xt,
            t=10.0,
            dt=0.1,
            num_iters=10
        ), 128),
        metric="hamming"
    )
)


print("pred: {:.4f}".format(nn_precision(base_n, n2)))

# plot_dist(ids0, ids1, ids2)
