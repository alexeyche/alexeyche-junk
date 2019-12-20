

import numpy as np
import scipy.sparse as sp
from util import shm, shl
from dataset import get_dataset
from util import *

from sklearn.datasets import load_digits
from sklearn.preprocessing import MinMaxScaler
from sklearn.datasets import load_iris

def xavier_init(fan_in, fan_out, const=1.0):
    low = -const * np.sqrt(6.0 / (fan_in + fan_out))
    high = const * np.sqrt(6.0 / (fan_in + fan_out))
    return (low + np.random.random((fan_in, fan_out)) * (high - low)).astype(np.float32)


def orthonormal_cost(F):
    return np.linalg.norm(np.dot(F.T, F) - np.eye(F.shape[1]))


def eigvec_cost(F, vh):
    return np.linalg.norm(np.dot(F, F.T) - np.dot(vh.T, vh))

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

def cost_psp(X, Y, W, M):
    T = X.shape[0]
    return (
        np.matrix.trace(
            - (4.0 / T) * np.dot(np.dot(X, W), Y.T)
            + (2.0 / T) * np.dot(np.dot(Y, M.T), Y.T)
        )
         + 2.0 * np.matrix.trace(np.dot(W.T, W))
         - np.matrix.trace(np.dot(M.T, M))
    )



np.random.seed(12)

iris = load_iris()
X = iris.data

# X = generate_matrix((2000, 10), 3)

T = X.shape[0]
dim = X.shape[1]

dst_dim = 2

tau = 0.1
dt = 1.0
# lrate = lambda it: 1.0 / (1000.0 + it)
lrate = lambda it: 0.001

rank = dst_dim


vh = np.linalg.svd(X)[2][:rank]

eiv, eig = np.linalg.eigh(np.cov(X.T))


Mx = xavier_init(dst_dim, dst_dim, const=0.1)
M = np.dot(Mx, Mx.T)

W = xavier_init(dim, dst_dim, const=0.1)

Y = np.zeros((T, dst_dim))
dY = np.zeros(Y.shape)

num_steps = 10000
metrics = np.zeros((num_steps, 5))
for it in range(num_steps):
    F = np.dot(np.linalg.inv(M), W.T).T
    Y = np.dot(X, F)

    Cx = np.dot(X.T, Y) / T
    Cy = np.dot(Y.T, Y) / T

    dW = 2.0 * (Cx - W)
    dM = Cy - M
    # dM = Cy - np.eye(M.shape[0])

    W += dt * lrate(it) * dW
    M += dt * lrate(it) * dM / tau

    metrics[it, 0:3] = (
        np.linalg.norm(dY),
        np.linalg.norm(dW),
        np.linalg.norm(dM)
    )
    metrics[it, 3] = eigvec_cost(F, vh)
    metrics[it, 4] = orthonormal_cost(F)

    if it % 2000 == 0:
        print("Iter {}, |dY| = {:.2f}, |dW| = {:.2f}, |dM| = {:.2f}, eig cost = {:.4f}, orth cost = {:.2f}".format(
            it, metrics[it, 0], metrics[it, 1], metrics[it, 2], metrics[it, 3], metrics[it, 4]
        ))


shl(
    metrics[500:, 0],
    metrics[500:, 1],
    metrics[500:, 2],
    metrics[500:, 3],
    metrics[500:, 4],
    labels=["|dY|", "|dW|", "|dM|", "eig cost", "orthonormal cost"]
)

# shs(Y, c=iris.target)

