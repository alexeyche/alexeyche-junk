
from poc.datasets import *
from poc.util import *
from sklearn.datasets.samples_generator import make_blobs

from scipy.sparse import linalg

def flatten(p):
    return [pp for param in p for pp in param]

def norm(W, axis=1):
    return W / np.linalg.norm(W, 2, axis=axis, keepdims=True)



input_size = 20
output_size = 20
weight_factor = 1.0
threshold = 0.1
layer_size = 200

sparsity = 0.95
batch_size = 300
beta = 0.0
num_iters = 0
K = layer_size // 20

centers = [[1, 1], [-1, -1], [1, -1]]
x, labels_true = make_blobs(n_samples=batch_size, centers=centers, cluster_std=0.5, random_state=0)
x = quantize_data(x, input_size)
x = np.concatenate((x, np.zeros((batch_size, layer_size-x.shape[1])),), 1)


W, _ = sparse_xavier_init(layer_size, layer_size, const=weight_factor, p=1.0-sparsity)
W = norm(W.todense())




u = np.zeros((batch_size, layer_size))
u[np.random.random((batch_size, layer_size))<np.expand_dims(np.tile(K/layer_size, batch_size), 1)] = 1.0

uh = np.zeros((num_iters, batch_size, layer_size))


u = np.random.random((batch_size, layer_size))


for ti in range(num_iters):
    new_u = x + np.dot(u, W)

    # W += beta * np.dot(u.T, new_u) / batch_size
    # W = norm(W)

    uh[ti] = new_u.copy()
    u = new_u

    ind = np.argpartition(u, -K, axis=1)[:, -K:]

    print(ti)
# shs(x, labels=(labels_true,))


