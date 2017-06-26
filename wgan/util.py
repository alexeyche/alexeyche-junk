
from matplotlib import pyplot as plt
import numpy as np
import math
import collections
import pylab

def shm(*matrices, **kwargs):
    plt.figure(figsize=kwargs.get("figsize", (10,7)))
    for m_id, matrix in enumerate(matrices):
        plt.subplot(len(matrices), 1, m_id+1)
        plt.imshow(np.squeeze(matrix).T, cmap='gray', origin='lower')
        plt.colorbar()

    if kwargs.get("file"):
        plt.savefig(kwargs["file"])
        plt.clf()
    elif kwargs.get("show", True):
        plt.show()


def smooth(signal, sigma=0.01, filter_size=50):
    lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
    return np.convolve(lf_filter, signal, mode="same")

def smooth_matrix(m, sigma=0.01, filter_size=50):
    res = np.zeros(m.shape)
    for dim_idx in xrange(m.shape[1]):
        res[:, dim_idx] = smooth(m[:, dim_idx], sigma, filter_size)
    return res

def shl(*vector, **kwargs):
    plt.figure(figsize=kwargs.get("figsize", (10,10)))
    
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

    for a in args:
        if make_pca and a.shape[1] > 2:
            import sklearn.decomposition as dec
            pca = dec.PCA(2)
            a = pca.fit(a).transform(a)
        
        if len(labels) > 0:
            plt.scatter(a[:,0], a[:,1], c=labels, cmap=pylab.cm.gist_rainbow)
        else:
            plt.scatter(a[:,0], a[:,1])
    
    if len(labels) > 0:
        plt.legend()

    if kwargs.get("file"):
        plt.savefig(kwargs["file"])
        plt.clf()
    else:
        plt.show()


def moving_average(a, n=3) :
    ret = np.cumsum(a, dtype=float)
    ret[n:] = ret[n:] - ret[:-n]
    v = ret[n - 1:] / n
    return np.pad(v, [0, n-1], 'constant')

def norm(data, return_denom=False):
    data_denom = np.sqrt(np.sum(data ** 2))
    data = data/data_denom
    if not return_denom:
        return data
    return data, data_denom



def generate_dct_dictionary(l, size):
    p = np.asarray(xrange(l))
    filters = np.zeros((l, size))
    for fi in xrange(size):
        filters[:, fi] = np.cos((np.pi * (2 * fi + 1) * p)/(2*l))
        filters[0, fi] *= 1.0/np.sqrt(2.0)
        # filters[fi, 1:] *= np.sqrt(2/l)
    return filters * np.sqrt(2.0/l)


def is_sequence(obj):
    if isinstance(obj, basestring):
        return False
    return isinstance(obj, collections.Sequence)


