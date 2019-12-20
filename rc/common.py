
import numpy as np

def heavyside(x):
    return np.where(x > 0.0, 1.0, 0.0)

def gen_wmap(c, l, r):
    wmap = np.zeros((c.N,)).astype(np.bool)
    wmap[l:r] = True
    return wmap

def synaptic_normalization(W):
    z = np.sum(np.abs(W), axis=0)
    z[z < 1e-6] = 1e-6
    W /= z[None, :]

def stdp(x0, x1, y0=None, y1=None):
    assert (y0 is not None) == (y1 is not None), \
        "y0 y1 should be both not none"

    if y0 is None and y1 is None:
        A = np.dot(x0.T, x1)
        return A - A.T

    return np.dot(x0.T, y1) - np.dot(x1.T, y0)

def bound_weights(W, M, c):
    W[:] = np.where(W < 0, 0.0, W)
    W[:] = np.where(W > c.upper_bound, c.upper_bound, W)
    M[:] = np.where(W <= 0.0, False, True)

def print_pressures(W, c):
    inp = gen_wmap(c, 0, c.Nu)
    exc = gen_wmap(c, c.Nu, c.Nu + c.Ne)
    out = gen_wmap(c, c.Nu + c.Ne, c.Nu + c.Ne + c.Nu)

    for i in range(c.N):
        x = np.zeros((c.batch_size, c.N))
        x[0, i] = 1.0
        a = np.dot(x, W)

        print("From n{} input p: {:.3f}, exc p: {:.3f}, output p: {:.3f}".format(
            i,
            np.sum(a[:, inp]),
            np.sum(a[:, exc]),
            np.sum(a[:, out])
        ))
