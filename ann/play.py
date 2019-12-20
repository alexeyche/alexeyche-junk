

import numpy as np
from matplotlib import pyplot as plt
import os
import time
from scipy.spatial.distance import pdist as scipy_pdist
import sklearn
from metrics import *
from sklearn.preprocessing import normalize
from sklearn.preprocessing import MinMaxScaler
from dataset import get_dataset
from util import Timing, get_faiss, shm
from models import running_knn_bruteforce

from sklearn.metrics.pairwise import euclidean_distances

from lsh import LSH
from bio_hash import BioHash



# D = get_dataset("glove-25-angular")
D = get_dataset("mnist-784-euclidean")

metric = euclidean

X_train = np.array(D["train"])
X_test = np.array(D["test"])

expected_dist = np.array(D["distances"])
nn = expected_dist.shape[1]


X_train = X_train - np.mean(X_train, 1, keepdims=True)
X_test = X_test - np.mean(X_test, 1, keepdims=True)

if metric is angular:
    X_train = normalize(X_train, axis=1, norm='l2')
    X_test = normalize(X_test, axis=1, norm='l2')


if X_train.dtype != np.float32:
    X_train = X_train.astype(np.float32)

if X_test.dtype != np.float32:
    X_test = X_test.astype(np.float32)


x = get_random(X_train, 10000)
xt = get_random(x, 1000)
nn = 200


_, idx0 = get_knn(x, xt, nn)

algo = BioHash(x.shape[1], 2000, 6.0 / 2000.0, 32)

# algo = LSH(x.shape[1], hash_length=32, binary=True, metric=metric)
# algo = LSH(x.shape[1], hash_length=32, bin_size=1.0, metric=metric)

bx = algo.get_bins(x)
bxt = algo.get_bins(xt)

_, idx1 = get_knn(bx, bxt, nn)


print(nn_precision(idx0, idx1))


f = np.asarray(bxt.sum(axis=0).tolist()[0])
plt.bar(np.arange(f.shape[0]), f[np.argsort(f)[::-1]]); plt.show()


