
import numpy as np
from sklearn.datasets import make_classification
import os
from util import smooth_batch_matrix, exp_filter

def to_sparse_ts(d, num_iters, at=10, kernel=exp_filter, sigma=0.01, filter_size=50):
    d_ts = np.zeros((num_iters,) + d.shape)
    d_ts[at] = d.copy()
    return smooth_batch_matrix(d_ts, kernel=kernel, sigma=sigma, filter_size=filter_size)


def get_toy_sparse_data(dest_dim, size, n_classes=2, seed=2):
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
    return quantize_data(x_values, dest_dim), y_values.astype(np.int32)

def quantize_data(x, dest_size):
    x_out = np.zeros((x.shape[0], dest_size))
    
    dim_size = x.shape[1]
    size_per_dim = dest_size/dim_size
    
    min_vals = np.min(x, 0)
    max_vals = np.max(x, 0)
    for xi in range(x.shape[0]):
        for di in range(dim_size):
            v01 = (x[xi, di] - min_vals[di]) / (max_vals[di] - min_vals[di])
            x_out[xi, int(di * size_per_dim + v01 * (size_per_dim-1))] = 1.0
    return x_out


def get_toy_data_baseline():
    return get_toy_data(4, 200, 2, 5)

def get_toy_data(dest_dim, size, n_classes=2, seed=2):
    x_values, y_values = make_classification(
        n_samples=size,
        n_features=dest_dim, 
        n_informative=dest_dim/2, 
        n_redundant=0, 
        n_repeated=0,
        n_clusters_per_class=2,
        n_classes=n_classes,
        random_state=seed
    )
    # return 0.01*np.log(1.0 + np.exp(x_values)), y_values.astype(np.int32)
    return x_values, y_values.astype(np.int32)

def get_toy_data_mclass():
    x_values, y_values = make_classification(
        n_samples=200,
        n_features=10, 
        n_informative=5, 
        n_redundant=0, 
        n_repeated=0,
        n_clusters_per_class=1,
        n_classes=10,
        scale=0.1,
        shift=5.0,
        random_state=2
    )
    return x_values, y_values.astype(np.int32)

def one_hot_encode(target_v, size=None):
    y_v = np.zeros((target_v.shape[0], size if not size is None else len(np.unique(target_v))))
    for cl_id, cl_v in enumerate(np.unique(target_v)):
        y_v[np.where(target_v==cl_v)[0], cl_id] = 1.0

    return y_v


class TaskType(object):
    CLASSIFICATION = "classification"
    REGRESSION = "regression"


class Dataset(object):
    @property
    def train_shape(self):
        raise NotImplementedError

    @property
    def test_shape(self):
        raise NotImplementedError

    @property
    def batch_size(self):        
        raise NotImplementedError

    def next_train_batch(self):
        raise NotImplementedError

    def next_test_batch(self):
        raise NotImplementedError

    @property
    def train_batch_size(self):
        raise NotImplementedError

    @property
    def test_batch_size(self):
        raise NotImplementedError

    @property
    def train_batches_num(self):
        return self.train_shape[0][0] // self.train_batch_size

    @property
    def test_batches_num(self):
        return self.test_shape[0][0] // self.test_batch_size

    @property
    def task_type(self):
        raise NotImplementedError

    @property
    def train_data(self):
        raise NotImplementedError
    
    @property
    def test_data(self):
        raise NotImplementedError

class MNISTDataset(Dataset):
    def __init__(self):
        from tensorflow.examples.tutorials.mnist import input_data

        self._data = input_data.read_data_sets(
            "{}/tmp/MNIST_data/".format(
                os.environ["HOME"]
                if "HOME" in os.environ else
                os.environ["HOMEPATH"]
            ),
            one_hot=True
        )
        self._train_batch_size = 200
        self._test_batch_size = 200
        self._i = 0

    @property
    def train_shape(self):
        return self._data.train.images.shape, self._data.train.labels.shape

    @property
    def test_shape(self):
        return self._data.test.images.shape, self._data.test.labels.shape

    def next_train_batch(self):
        tup_to_return = (
            self._data.train.images[self._i * self._train_batch_size:(self._i + 1) * self.train_batch_size], 
            self._data.train.labels[self._i * self._train_batch_size:(self._i + 1) * self.train_batch_size]
        )
        self._i += 1
        if self._i >= self.train_batches_num:
            self._i = 0
        return tup_to_return
        
    def next_test_batch(self):
        tup_to_return = (
            self._data.test.images[self._i * self._test_batch_size:(self._i + 1) * self._test_batch_size], 
            self._data.test.labels[self._i * self._test_batch_size:(self._i + 1) * self._test_batch_size]
        )
        self._i += 1
        if self._i >= self.test_batches_num:
            self._i = 0
        return tup_to_return

    @property
    def train_batch_size(self):
        return self._train_batch_size

    @property
    def test_batch_size(self):
        return self._test_batch_size

    # @property
    # def train_batches_num(self):
    #     return self._train_batch_size

    # @property
    # def test_batches_num(self):
    #     return 10 # TODO

    @property
    def task_type(self):
        return TaskType.CLASSIFICATION

    @property
    def train_data(self):        
        return self._data.train.images, self._data.train.labels
    
    @property
    def test_data(self):        
        return self._data.test.images, self._data.test.labels

class ToyDataset(Dataset):
    def __init__(self):
        x_v, target_v = get_toy_data_baseline()
        y_v = one_hot_encode(target_v)

        test_prop = x_v.shape[0]/5

        self._xt_v = x_v[:test_prop]
        self._yt_v = y_v[:test_prop]

        self._x_v = x_v[test_prop:]
        self._y_v = y_v[test_prop:]
        
    @property
    def train_shape(self):
        return self._x_v.shape, self._y_v.shape

    @property
    def test_shape(self):
        return self._xt_v.shape, self._yt_v.shape

    def next_train_batch(self):
        return self._x_v, self._y_v

    def next_test_batch(self):
        return self._xt_v, self._yt_v

    @property
    def train_batch_size(self):
        return self._x_v.shape[0]

    @property
    def test_batch_size(self):
        return self._xt_v.shape[0]

    @property
    def task_type(self):
        return TaskType.CLASSIFICATION

    @property
    def train_data(self):        
        return self._x_v, self._y_v
    
    @property
    def test_data(self):        
        return self._xt_v, self._yt_v


class XorDatasetSmall(Dataset):
    def __init__(self):
        self._x_v = np.asarray([
            [0.0, 0.0],
            [0.0, 1.0],
            [1.0, 0.0],
            [1.0, 1.0]
        ], dtype=np.float32)
        self._y_v = np.asarray([
            [0.0],
            [1.0],
            [1.0],
            [0.0]
        ], dtype=np.float32)

    @property
    def train_shape(self):
        return self._x_v.shape, self._y_v.shape

    @property
    def test_shape(self):
        return self._x_v.shape, self._y_v.shape

    def next_train_batch(self):
        return self._x_v, self._y_v

    def next_test_batch(self):
        return self._x_v, self._y_v

    @property
    def train_batch_size(self):
        return self._x_v.shape[0]

    @property
    def test_batch_size(self):
        return self._x_v.shape[0]

    @property
    def task_type(self):
        return TaskType.REGRESSION

    @property
    def train_data(self):        
        return self._x_v, self._y_v
    
    @property
    def test_data(self):        
        return self._x_v, self._y_v


class XorDataset(Dataset):
    def __init__(self):
        sd = 0.2
        x_v = np.concatenate([
            sd*np.random.randn(200, 2).astype(np.float32),
            [0.0, 1.0] + sd*np.random.randn(200, 2).astype(np.float32),
            [1.0, 0.0] + sd*np.random.randn(200, 2).astype(np.float32),
            [1.0, 1.0] + sd*np.random.randn(200, 2).astype(np.float32),
        ], )
        y_v = one_hot_encode(np.concatenate([
            np.zeros((200,), dtype=np.float32),
            np.ones((200,), dtype=np.float32),
            np.ones((200,), dtype=np.float32),
            np.zeros((200,), dtype=np.float32),
        ]), 2)

        ids = np.random.permutation(x_v.shape[0])
        x_v = x_v[ids]
        y_v = y_v[ids]

        test_prop = x_v.shape[0] // 4

        self._xt_v = x_v[:test_prop]
        self._yt_v = y_v[:test_prop]

        self._x_v = x_v[test_prop:]
        self._y_v = y_v[test_prop:]

        self._train_batch_size = 200
        self._test_batch_size = 200
        self._i = 0
        self._it = 0

    @property
    def train_shape(self):
        return self._x_v.shape, self._y_v.shape


    @property
    def test_shape(self):
        return self._xt_v.shape, self._yt_v.shape


    def next_train_batch(self):
        tup_to_return = (
            self._x_v[self._i * self._train_batch_size:(self._i + 1) * self.train_batch_size],
            self._y_v[self._i * self._train_batch_size:(self._i + 1) * self.train_batch_size]
        )
        self._i += 1
        if self._i >= self.train_batches_num:
            self._i = 0
        return tup_to_return


    def next_test_batch(self):
        tup_to_return = (
            self._xt_v[self._it * self._test_batch_size:(self._it + 1) * self._test_batch_size],
            self._yt_v[self._it * self._test_batch_size:(self._it + 1) * self._test_batch_size]
        )
        self._it += 1
        if self._it >= self.test_batches_num:
            self._it = 0
        return tup_to_return


    @property
    def train_batch_size(self):
        return self._train_batch_size


    @property
    def test_batch_size(self):
        return self._test_batch_size


    @property
    def train_batches_num(self):
        return self._x_v.shape[0]/self._train_batch_size


    @property
    def test_batches_num(self):
        return self._xt_v.shape[0] / self._test_batch_size


    @property
    def task_type(self):
        return TaskType.CLASSIFICATION


    @property
    def train_data(self):
        return self._x_v, self._y_v


    @property
    def test_data(self):
        return self._xt_v, self._yt_v


def whiten(X,fudge=1E-18):
   # the matrix X should be observations-by-components

   # get the covariance matrix
   Xcov = np.dot(X.T,X)

   # eigenvalue decomposition of the covariance matrix
   d, V = np.linalg.eigh(Xcov)

   # a fudge factor can be used so that eigenvectors associated with
   # small eigenvalues do not get overamplified.
   D = np.diag(1. / np.sqrt(d+fudge))

   # whitening matrix
   W = np.dot(np.dot(V, D), V.T)

   # multiply by the whitening matrix
   X_white = np.dot(X, W)

   return X_white, W


class FacesDataset(Dataset):
    def __init__(self, batch_size, max_patches=50, patch_size=(20, 20), images_num=None, rng=None):
        from sklearn import datasets as sklearn_datasets
        from sklearn.feature_extraction.image import extract_patches_2d

        self._train_batch_size = batch_size
        self._test_batch_size = batch_size

        rng = rng if not rng is None else np.random.RandomState(12)

        faces = sklearn_datasets.fetch_olivetti_faces()
        images_num = images_num if not images_num is None else faces.images.shape[0]

        x_v = np.zeros((max_patches * images_num, patch_size[0]*patch_size[1]))
        classes = np.zeros((max_patches * images_num,))
        
        for img_id, img in enumerate(faces.images):
            if img_id >= images_num:
                break

            patches_id = ((img_id * max_patches),((img_id+1) * max_patches))
            
            x_v[patches_id[0]:patches_id[1], :] = extract_patches_2d(
                img, 
                patch_size, 
                max_patches=max_patches, 
                random_state=rng
            ).reshape((max_patches, patch_size[0]*patch_size[1]))
            
            classes[patches_id[0]:patches_id[1]] = faces.target[img_id]
        
        y_v = one_hot_encode(classes)
        
        test_prop = x_v.shape[0]/5

        self._xt_v = x_v
        self._yt_v = y_v

        self._x_v = x_v
        self._y_v = y_v
        self._i = 0

        self._x_v -= np.mean(self._x_v, axis=0)
        self._x_v /= np.std(self._x_v, axis=0)
        self._x_v *= 0.1
        # self._x_v = whiten(self._x_v)[0]
        
    @property
    def train_shape(self):
        # return (self._train_batch_size, self._x_v.shape[1]), (self._train_batch_size, self._y_v.shape[1])
        return self._x_v.shape, self._y_v.shape

    @property
    def test_shape(self):
        # return (self._test_batch_size, self._xt_v.shape[1]), (self._test_batch_size, self._yt_v.shape[1])
        return self._xt_v.shape, self._yt_v.shape

    def next_train_batch(self):
        tup_to_return = (
            self._x_v[self._i * self._train_batch_size:(self._i + 1) * self._train_batch_size], 
            self._y_v[self._i * self._train_batch_size:(self._i + 1) * self._train_batch_size]
        )
        self._i += 1
        if self._i >= self.train_batches_num:
            self._i = 0
        return tup_to_return
        
    def next_test_batch(self):
        tup_to_return = (
            self._xt_v[self._i * self._test_batch_size:(self._i + 1) * self._test_batch_size], 
            self._yt_v[self._i * self._test_batch_size:(self._i + 1) * self._test_batch_size]
        )
        self._i += 1
        if self._i >= self.test_batches_num:
            self._i = 0
        return tup_to_return

    @property
    def train_batch_size(self):
        return self._train_batch_size

    @property
    def test_batch_size(self):
        return self._test_batch_size

    @property
    def task_type(self):
        return TaskType.CLASSIFICATION

    @property
    def train_data(self):        
        return self._x_v, self._y_v
    
    @property
    def test_data(self):        
        return self._xt_v, self._yt_v


