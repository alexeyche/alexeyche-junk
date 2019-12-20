
from matplotlib import pyplot as plt
import numpy as np
import time
from dataset import get_workdir
from os.path import join as pj
import sys

def plot_wrapper(fn):
    def wrapper(*args, **kwargs):
        ncols = kwargs.get("ncols", 1)
        id = kwargs.get("id", 0)
        nrows = len(args)

        if id == 0:
            plt.figure(figsize=kwargs.get("figsize", (25, 10)))

        for a_id, a in enumerate(args):
            plt.subplot(nrows, ncols, nrows * id + a_id + 1)
            fn(a, **kwargs)

        if kwargs.get("file"):
            plt.savefig(kwargs["file"])
            plt.clf()
        elif kwargs.get("show", True) and id + 1 == ncols:
            plt.show()

    return wrapper

def get_random(x, n):
    return x[np.random.choice(range(x.shape[0]), n)]


@plot_wrapper
def shm(matrix, **kwargs):
    plt.imshow(np.squeeze(matrix), cmap='gray', origin='lower')
    plt.colorbar()

class Timing(object):
    def __init__(self, message, verbose=True):
        self.message = message
        self.verbose = verbose
        self.t0 = None
        self.t1 = None

    def __enter__(self):
        if self.verbose:
            print("Running `{}` ...".format(self.message))
        self.t0 = time.time()
        return self

    def __exit__(self, *args):
        self.t1 = time.time()
        if self.verbose:
            print("Done. {:.2f}s".format(self.t1 - self.t0))



def get_faiss():
    d = pj(get_workdir(), "faiss", "python")
    if sys.path[0] != d:
        sys.path.insert(0, d)
    import faiss
    return faiss

def to_dec(xx):
    return int("0b{}".format("".join(["1" if xxx else "0" for xxx in xx])), 2)


def bool_array_to_dec(x):
    return np.asarray(list(map(to_dec, x))).reshape((x.shape[0], 1))


def shl(*vector, **kwargs):
    plt.figure(figsize=kwargs.get("figsize", (25, 10)))

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
    labels = kwargs.get("c", [])
    make_pca = kwargs.get("make_pca", True)
    assert len(args) == 1
    a = args[0]

    plt.figure(figsize=kwargs.get("figsize", (25, 10)))
    if len(labels) > 0:
        if make_pca and a.shape[1] > 2:
            import sklearn.decomposition as dec
            pca = dec.PCA(2)
            a = pca.fit(a).transform(a)

        plt.scatter(a[:, 0], a[:, 1], c=labels)
        plt.legend()
    else:
        plt.scatter(a[:, 0], a[:, 1])

    if kwargs.get("file"):
        plt.savefig(kwargs["file"])
        plt.clf()
    elif kwargs.get("show", True):
        plt.show()

