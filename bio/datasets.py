import numpy as np
from sklearn.datasets import make_classification

def get_toy_data(size=2000, n_classes=2, seed=2):
    x_values, y_values = make_classification(
        n_samples=size,
        n_features=2, 
        n_informative=2, 
        n_redundant=0, 
        n_repeated=0,
        n_clusters_per_class=2,
        n_classes=n_classes,
        scale=0.1,
        shift=5.0,
        random_state=seed
    )
    return quantize_data(x_values, 50), y_values.astype(np.int32)



def quantize_data(x, dest_size):
    x_out = np.zeros((x.shape[0], dest_size))
    
    dim_size = x.shape[1]
    size_per_dim = dest_size/dim_size
    
    min_vals = np.min(x, 0)
    max_vals = np.max(x, 0)
    for xi in xrange(x.shape[0]):
        for di in xrange(dim_size):
            v01 = (x[xi, di] - min_vals[di]) / (max_vals[di] - min_vals[di])
            x_out[xi, int(di * size_per_dim + v01 * (size_per_dim-1))] = 1.0
    return x_out