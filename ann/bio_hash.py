
import numpy as np
import scipy.sparse as sp
from util import shm
from dataset import get_dataset
from sklearn.preprocessing import normalize

def build_sparse(rows, cols, p):
    Msize = int(rows * cols * p * 2)
    M = (
        np.random.randint(0, rows * cols, size=Msize)
            .reshape((int(Msize / 2), 2))
    )
    return sp.csr_matrix(
        (
            np.ones(M.shape[0]),
            (M[:, 0] // cols, M[:, 1] // rows)
        ),
        shape=(rows, cols)
    )


def get_top_active_indices(U, top_k):
    if isinstance(U, np.ndarray):
        U = sp.csr_matrix(U)

    ans = np.zeros((U.shape[0], top_k), dtype=np.uint32)
    for ri in range(U.shape[0]):
        r = U.getrow(ri)
        ans[ri] = r.indices[np.argsort(r.data)[::-1]][:top_k]

    ans_row_ids = (
        np.tile(np.arange(U.shape[0]), top_k)
            .reshape((top_k, U.shape[0]))
            .T
            .reshape(-1)
    )
    ans_col_ids = ans.reshape(-1)
    return sp.csr_matrix(
        (
            np.ones(ans.shape[0] * ans.shape[1]),
            (ans_row_ids, ans_col_ids)
        ),
        shape=(ans.shape[0], U.shape[1])
    )

class BioHash(object):
    def __init__(self, dim, K, p, top_k):
        self.W = build_sparse(K, dim, p)
        self.K = K
        self.dim = dim
        self.top_k = top_k

    def get_bins_binary(self, x):
        assert x.shape[1] == self.dim

        U = sp.csr_matrix(self.W.dot(x.T).T)

        return get_top_active_indices(U, self.top_k)

    def act(self, U, t):
        A = U.copy()
        A -= t
        A = np.maximum(A, 0.0)
        return A

    # def act(self, U, t):
    #     return get_top_active_indices(U, self.top_k).todense()


    def get_bins(self, x, t):
        assert x.shape[1] == self.dim

        U = self.W.dot(x.T).T
        A = self.act(U, t)
        # return get_top_active_indices(A, self.top_k)
        return A

    def get_bins_dyn(self, x, t, dt, num_iters, lrate=0.0, num_epochs=1):
        # A = sp.csr_matrix((x.shape[0], self.K)).todense()
        # U = sp.csr_matrix((x.shape[0], self.K)).todense()
        if isinstance(self.W, sp.csr_matrix):
            self.W = self.W.todense()


        self.rh = np.zeros((num_epochs,))

        for epoch in range(num_epochs):
            self.Ah = np.zeros((num_iters, x.shape[0], self.K))

            A = np.zeros((x.shape[0], self.K))
            U = np.zeros((x.shape[0], self.K))

            for it in range(num_iters):
                # p = A.dot(self.W.todense())
                p = A.dot(self.W)

                # shm(p[1].reshape((28, 28)))

                r = x - p
                U += dt * (self.W.dot(r.T).T - U)

                A = self.act(U, t)

                self.Ah[it] = A.copy()

                # A.eliminate_zeros()

                # A = get_top_active_indices(sp.csr_matrix(U), 128)
                # if it % 10 == 0:

            dW = np.dot(A.T, r) / self.K / self.dim

            self.W += lrate * dW

            self.rh[epoch] = np.linalg.norm(r)

            print("Epoch {} {:.4f}".format(epoch, self.rh[epoch]))

        # return get_top_active_indices(A, self.top_k)
        return A



class BioRecHash(object):
    def __init__(self, dim, K, p, p_rec, top_k, scale):
        self.W = build_sparse(K, dim, p) * scale
        self.R = build_sparse(K, K, p_rec) * scale

        self.K = K
        self.dim = dim
        self.top_k = top_k

    def act(self, U, t):
        A = U.copy()
        A -= t
        A = np.maximum(A, 0.0)
        return A


    def get_bins_dyn(self, x, t, dt, num_iters, lrate=0.0, num_epochs=1):
        # A = sp.csr_matrix((x.shape[0], self.K)).todense()
        # U = sp.csr_matrix((x.shape[0], self.K)).todense()
        if isinstance(self.W, sp.csr_matrix):
            self.W = self.W.todense()
        if isinstance(self.R, sp.csr_matrix):
            # self.R = self.R.todense()
            self.R = self.W.dot(self.W.T)

        self.rh = np.zeros((num_epochs,))

        for epoch in range(num_epochs):
            self.Ah = np.zeros((num_iters, x.shape[0], self.K))

            A = np.zeros((x.shape[0], self.K))
            U = np.zeros((x.shape[0], self.K))

            for it in range(num_iters):
                # p = A.dot(self.W.todense())

                dU = self.W.dot(x.T).T - self.R.dot(A.T).T - U
                U += dt * dU

                A = self.act(U, t)

                self.Ah[it] = A.copy()

                # A.eliminate_zeros()

                # A = get_top_active_indices(sp.csr_matrix(U), 128)
                # if it % 10 == 0:
                # print("Iter {} {:.4f}".format(it, np.linalg.norm(x - A.dot(self.W))))

            # p = A.dot(self.W)
            # shm(p[1].reshape((28, 28)))

            # r = x - p

            # dW = np.dot(A.T, r) / self.K / self.dim
            dW = np.dot(A.T, x) / self.K / self.dim - self.W
            dR = np.dot(A.T, A) / self.K / self.K - np.eye(self.K)

            self.W += lrate * dW
            self.R += lrate * dR

            self.rh[epoch] = np.linalg.norm(dW)

            print("Epoch {} {:.4f} {:.4f} {:.4f}".format(
                epoch, np.linalg.norm(dW), np.linalg.norm(dR),
                np.linalg.norm(np.dot(A.T, A) - np.eye(self.K))
            ))

        # return get_top_active_indices(A, self.top_k)
        return A, dW, np.dot(A.T, x) / self.K / self.dim, np.dot(A.T, A) / self.K / self.K

