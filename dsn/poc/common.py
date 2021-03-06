import numpy as np


def safe_log(x):
    return np.log(x + 1e-05)


def threshold_k_10(u):
    K = int(u.shape[1] * 0.1)
    return threshold_k(u, K)

def threshold_k(u, K):
    a = np.zeros(u.shape)
    batch_size = u.shape[0]

    bidx = np.arange(0, batch_size)
    ind = np.argpartition(u, -K, axis=1)[:, -K:]

    a[np.expand_dims(bidx, 1), ind] = 1.0
    a[np.where(np.abs(u) < 1e-10)] = 0.0
    return a

def threshold_p(u, p):
    K = int(u.shape[1] * p)
    return threshold_k(u, K)


def ltd(a, a_mp):
    a_silent_mp = np.where(a_mp < 1e-10)
    a_ltd = np.zeros((a.shape))
    a_ltd[a_silent_mp] = a[a_silent_mp]
    return a_ltd


sigmoid = lambda x: 1.0/(1.0 + np.exp(-x))
def sigmoid_prime(x):
    v = sigmoid(x)
    return v * (1.0 - v)

sigmoid_inv = lambda x: np.log(x/(1.0 - x + 1e-09) + 1e-09)

relu = lambda x: np.maximum(x, 0.0)
def relu_prime(x):
    dadx = np.zeros(x.shape)
    # dadx[np.where(x > 0.0)] = 1.0
    a = relu(x)
    dadx[np.where(a > 0.0)] = 1.0
    return dadx


def threshold(x, threshold_value = 0.0):
    a = np.zeros(x.shape)
    a[np.where(x > threshold_value)] = 1.0
    return a


def threshold_prime(x, threshold_value = 0.7):
    return 1.0/np.square(1.0 + np.abs(x - threshold_value))

linear = lambda x: x
linear_prime = lambda x: 1.0


def number_of_equal_act(a, a_t):
    a_t_m = a_t.copy()
    a_t_m[np.where(np.abs(a_t_m) < 1e-10)] = -1

    a_m = a.copy()
    a_m[np.where(np.abs(a_m) < 1e-10)] = -10

    return np.mean(np.equal(a_t_m, a_m)) / np.mean(a_t)
