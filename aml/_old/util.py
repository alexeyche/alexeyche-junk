
from matplotlib import pyplot as plt
import numpy as np
import math
import collections
from scipy import signal
import pandas as pd


from scipy.sparse import csr_matrix, coo_matrix


DEFAULT_FIG_SIZE = (7, 7)

def flatten(p):
    return [pp for param in p for pp in param]

def random_orth(shape):
    import scipy as sp
    max_shape = max(shape[0], shape[1])

    shape_to_gen = (max_shape, max_shape)
    Q, _ = sp.linalg.qr(np.random.randn(*shape_to_gen))

    return Q[:shape[0], :shape[1]]

def random_pos_orth(shape):
    rows, cols = shape
    A = np.zeros((rows, cols), dtype=np.float32)

    if rows <= cols:
        v = np.arange(0, cols)
        ss = cols // rows
        for rid in range(rows):
            cid = np.random.choice(v, ss)
            v = np.setdiff1d(v, cid)
            Av = np.ones(len(cid))
            A[rid, cid] = Av
        return A / (ss / np.sqrt(ss))
    else:
        ss = rows // cols

        vr = np.arange(0, rows)
        vc = np.arange(0, cols)
        rids = np.random.choice(vr, rows)

        for rid in rids:
            if len(vc) == 0:
                break
            cid = np.random.choice(vc, 1)
            vc = np.setdiff1d(vc, cid)
            Av = np.ones(len(cid))
            A[rid, cid] = Av
        return A




def random_pos_sparse(shape, p):
    W = np.random.random(shape).astype(np.float32)
    mask = np.random.random(shape) >= p
    return W * mask.astype(np.float32)


def random_sparse(shape, p):
    W = np.random.random(shape).astype(np.float32) - 0.5
    mask = np.random.random(shape) >= p
    return W * mask.astype(np.float32)


def xavier_init(fan_in, fan_out, const=1.0):
    low = -const * np.sqrt(6.0 / (fan_in + fan_out))
    high = const * np.sqrt(6.0 / (fan_in + fan_out))
    return (
        (low + np.random.random((fan_in, fan_out)) * (high - low)).astype(np.float32),
        (low + np.random.random((fan_out,)) * (high - low)).astype(np.float32)
    )


def sparse_xavier_init(fan_in, fan_out, const=1.0, p=0.1):
    W, b = xavier_init(fan_in, fan_out, const)
    mask = np.random.random((fan_in, fan_out)) < p

    W *= mask.astype(np.float32)
    print("Sparsity level is {:.4f}".format(np.mean(np.equal(W, 0.0))))

    return csr_matrix(W), b



def plot_wrapper(fn):
    def wrapper(*args, **kwargs):
        ncols = kwargs.get("ncols", 1)
        id = kwargs.get("id", 0)
        nrows = len(args)

        if id == 0:
            plt.figure(figsize=kwargs.get("figsize", DEFAULT_FIG_SIZE))

        for a_id, a in enumerate(args):
            plt.subplot(nrows, ncols, nrows * id + a_id + 1)
            fn(a, **kwargs)

        if kwargs.get("file"):
            plt.savefig(kwargs["file"])
            plt.clf()
        elif kwargs.get("show", True) and id + 1 == ncols:
            plt.show()

    return wrapper



@plot_wrapper
def shm(matrix, **kwargs):
    plt.imshow(np.squeeze(matrix).T, cmap='gray', origin='lower')
    plt.colorbar()




def shl(*vector, **kwargs):
    plt.figure(figsize=kwargs.get("figsize", DEFAULT_FIG_SIZE))

    labels = kwargs.get("labels", [])
    for id, v in enumerate(vector):
        if len(labels) > 0:
            plt.plot(np.squeeze(v), label=labels[id])
        else:
            plt.plot(np.squeeze(v))

    if len(labels) > 0:
        plt.legend()

    if not kwargs.get("title") is None:
        plt.suptitle(kwargs["title"])

    if kwargs.get("file"):
        plt.savefig(kwargs["file"])
        plt.clf()
    elif kwargs.get("show", True):
        plt.show()

def shs(*args, **kwargs):
    labels = kwargs.get("labels", [])
    make_pca = kwargs.get("make_pca", True)

    plt.figure(figsize=kwargs.get("figsize", DEFAULT_FIG_SIZE))

    for id, a in enumerate(args):
        if make_pca and a.shape[1] > 2:
            import sklearn.decomposition as dec
            pca = dec.PCA(2)
            a = pca.fit(a).transform(a)

        if len(labels) > 0:
            plt.scatter(a[:, 0], a[:, 1], c=labels[id])
        else:
            plt.scatter(a[:, 0], a[:, 1])

    if len(labels) > 0:
        plt.legend()

    if kwargs.get("file"):
        plt.savefig(kwargs["file"])
        plt.clf()
    elif kwargs.get("show", True):
        plt.show()


def shp(*args, **kwargs):
    fs = kwargs.get("fs", 100)
    f, Pxx_den = signal.periodogram(args[0], fs)

    plt.semilogy(f, Pxx_den)
    plt.ylim([1e-7, 1e2])
    plt.xlabel('frequency [Hz]')
    plt.ylabel('PSD [V**2/Hz]')
    plt.show()




def is_sequence(obj):
    if isinstance(obj, str):
        return False
    return isinstance(obj, collections.Sequence)


