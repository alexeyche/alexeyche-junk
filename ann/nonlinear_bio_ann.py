

import numpy as np
import scipy.sparse as sp
from util import shm, shl
from dataset import get_dataset
from util import *
from metrics import *
from bio_hash import get_top_active_indices, BioHash

from sklearn.datasets import load_digits
from sklearn.preprocessing import MinMaxScaler
from sklearn.preprocessing import StandardScaler
from sklearn.datasets import load_iris

def xavier_init(fan_in, fan_out, const=1.0):
    low = -const * np.sqrt(6.0 / (fan_in + fan_out))
    high = const * np.sqrt(6.0 / (fan_in + fan_out))
    return (low + np.random.random((fan_in, fan_out)) * (high - low)).astype(np.float32)

def generate_matrix(shape, rank):
    U = np.random.randn(shape[0], rank)
    V = np.random.randn(rank, shape[1])

    S = np.diag([
        0.1 * np.sqrt(r * shape[0])
        for r in reversed(range(1, rank + 1))
    ])

    X = np.dot(np.dot(U, S), V)
    assert rank == np.linalg.matrix_rank(X)
    return X

def norm(m, coef=0.1, ord=2):
    return coef * m / np.linalg.norm(m, ord=ord, axis=0)

# np.random.seed(12)

# iris = load_iris()
# X = iris.data

D = get_dataset("mnist-784-euclidean")
X = get_random(np.array(D["train"]), 5000)
Xt = get_random(X, 500)

dst_dim = 500

# X = generate_matrix((2000, 10), 3)

X = X - np.mean(X, axis=0)
Xt = Xt - np.mean(Xt, axis=0)

# X = StandardScaler().fit_transform(X)
# Xt = StandardScaler().fit_transform(Xt)

nn = 50

base_n, _, _ = calc_ids(X, Xt, nn, lambda x: x, metric="euclidean")


algo = BioHash(
    dim=X.shape[1],
    K=dst_dim,
    p=2.0 / dst_dim,
    top_k=128
)


# n0, ids0, test_ids0 = (
#     calc_ids(
#         X,
#         Xt,
#         nn,
#         lambda x: algo.get_bins_binary(x),
#         metric="euclidean"
#     )
# )

n1, ids1, test_ids1 = (
    calc_ids(
        X,
        Xt,
        nn,
        lambda x: algo.get_bins(x, t=0.0),
        metric="euclidean"
    )
)

# print("binary: {:.4f}".format(nn_precision(base_n, n0)))
print("dot: {:.4f}".format(nn_precision(base_n, n1)))



def run_bio_hash(X, num_steps, dt, tau, lrate_cb, params=None, activity=None):
    T = X.shape[0]
    dim = X.shape[1]

    # lrate = lambda it: 1.0 / (1000.0 + it)
    # lrate = lambda it: 0.01
    # lrate = lambda it: 0.0

    if params is None:
        Mx = xavier_init(dst_dim, dst_dim, const=0.1)
        M = np.dot(Mx, Mx.T)

        W = xavier_init(dim, dst_dim, const=0.1)
        # M = np.dot(W.T, W)

        W = norm(W)
        M = norm(M)
    else:
        W, M = params

    if activity is None:
        Y = np.zeros((T, dst_dim))
        A = np.zeros((T, dst_dim))
    else:
        Y, A = activity

    metrics = np.zeros((num_steps, 3))
    for it in range(num_steps):
        dY = np.dot(X, W) - np.dot(A, M) - Y
        Y += dt * dY

        A = np.maximum(Y, 0.0)

        Cx = np.dot(X.T, A) / T
        Ca = np.dot(A.T, A) / T

        dW = norm(Cx) - W
        dM = norm(Ca) - M
        # dM = norm(Ca) - norm(np.eye(M.shape[0]))

        W += dt * lrate_cb(it) * dW
        M += dt * lrate_cb(it) * dM / tau

        W = norm(W)
        M = norm(M)

        metrics[it, 0:3] = (
            np.linalg.norm(dY),
            np.linalg.norm(dW),
            np.linalg.norm(dM)
        )

        if it % 10 == 0:
            print("Iter {}, |dY| = {:.2f}, |dW| = {:.2f}, |dM| = {:.2f}".format(
                it, metrics[it, 0], metrics[it, 1], metrics[it, 2]
            ))

    return A, (W, M), (Y, A)

params = None
activity = None
A, params, activity = (
    run_bio_hash(
        X,
        300,
        0.5,
        0.1,
        lambda it: 0.01,
        params,
        activity
    )
)

At, _, _ = run_bio_hash(
    Xt,
    50,
    0.5,
    0.1,
    lambda it: 0.0,
    params
)

_, n2 = get_knn(A, At, nn)

W, M = params

kt = lambda id: Xt[id].reshape((28, 28))
k = lambda id: X[id].reshape((28, 28))

print("bio: {:.4f}".format(nn_precision(base_n, n2)))





# print("bio: {:.4f}".format(nn_precision(base_n, n2)))

# shl(
#     metrics[:, 0],
#     metrics[:, 1],
#     metrics[:, 2],
#     labels=["|dY|", "|dW|", "|dM|"]
# )


# shs(Y, c=iris.target)

