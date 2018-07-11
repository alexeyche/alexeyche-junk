
from poc.datasets import *
from poc.util import *
from sklearn.datasets.samples_generator import make_blobs


def norm(W):
    delim = np.linalg.norm(W, 2, axis=0, keepdims=True)
    delim[np.where(delim == 0)] = 1.0
    return W / delim


input_size = 20
output_size = 20
weight_factor = 0.1
layer_size = 200

sparsity = 0.99
batch_size = 300
beta = 0.01
K = 20
dt = 0.01


def act(u):
    a = np.zeros(u.shape)
    batch_size = u.shape[0]

    bidx = np.arange(0, batch_size)
    ind = np.argpartition(u, -K, axis=1)[:, -K:]

    a[np.expand_dims(bidx, 1), ind] = 1.0
    a[np.where(np.abs(u) < 1e-10)] = 0.0
    return a


np.random.seed(4)



centers = [[1, 1], [-1, -1], [1, -1]]
x, labels_true = make_blobs(n_samples=batch_size, centers=centers, cluster_std=0.5, random_state=0)
x = quantize_data(x, input_size)


W0, _ = sparse_xavier_init(input_size, layer_size, const=weight_factor, p=1.0-sparsity)
# W0 = norm(np.asarray(W0.todense()))
W0 = np.asarray(W0.todense())

W1, _ = sparse_xavier_init(layer_size, layer_size, const=weight_factor, p=1.0-sparsity)
# W1 = norm(np.asarray(W1.todense()))
W1 = np.asarray(W1.todense())

W2, _ = sparse_xavier_init(layer_size, layer_size, const=weight_factor, p=1.0-sparsity)
# W2 = norm(np.asarray(W2.todense()))
W2 = np.asarray(W2.todense())

lrate = 0.00001

epochs = 3

u0h = np.zeros((epochs, batch_size, layer_size))
u1h = np.zeros((epochs, batch_size, layer_size))
u2h = np.zeros((epochs, batch_size, layer_size))

a0h = np.zeros((epochs, batch_size, layer_size))
a1h = np.zeros((epochs, batch_size, layer_size))
a2h = np.zeros((epochs, batch_size, layer_size))

for epoch in range(epochs):
    u0 = np.dot(x, W0)
    a0 = act(u0)

    u1 = np.dot(a0, W1)
    a1 = act(u1)

    u2 = np.dot(a1, W2)
    a2 = act(u2)

    dW0 = np.dot(x.T, a0) / batch_size
    dW1 = np.dot(a0.T, a1) / batch_size
    dW2 = np.dot(a1.T, a2) / batch_size

    W0 += lrate * dW0
    W1 += lrate * dW1
    W2 += lrate * dW2

    # W0 = norm(W0)
    # W1 = norm(W1)
    # W2 = norm(W2)

    u0h[epoch] = u0.copy()
    u1h[epoch] = u1.copy()
    u2h[epoch] = u2.copy()

    a0h[epoch] = a0.copy()
    a1h[epoch] = a1.copy()
    a2h[epoch] = a2.copy()

    print("{} {:.4f} {:.4f} {:.4f}".format(
        epoch,
        np.linalg.norm(dW0),
        np.linalg.norm(dW1),
        np.linalg.norm(dW2),
    ))