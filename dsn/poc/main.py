
from poc.datasets import *
from poc.util import *
from sklearn.datasets.samples_generator import make_blobs

def flatten(p):
    return [pp for param in p for pp in param]


input_size = 20
output_size = 20
weight_factor = 1.0
threshold = 0.1
layer_size = 200

sparsity = 0.95
batch_size = 300
dt = 0.2
num_iters = 100


centers = [[1, 1], [-1, -1], [1, -1]]
x, labels_true = make_blobs(n_samples=batch_size, centers=centers, cluster_std=0.5, random_state=0)
x = quantize_data(x, input_size)




W, _ = sparse_xavier_init(layer_size, layer_size, const=weight_factor, p=1.0-sparsity)


u = np.zeros((batch_size, layer_size))
uh = np.zeros((num_iters, batch_size, layer_size))



for ti in xrange(num_iters):
	u[:, :input_size] = x


	u += dt * (W.dot(u.T).T - u)

	uh[ti] = u.copy()

# shs(x, labels=(labels_true,))


