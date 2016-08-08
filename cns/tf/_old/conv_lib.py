
import numpy as np
import scipy
import scipy.sparse
import cPickle as pkl


class SparseAcoustic(object):
    def __init__(self, dense, data_denom):
        self.data = scipy.sparse.csr_matrix(dense)
        self.data_denom = data_denom

    def serialize(self, dst):
        with open(dst, "wb") as dst_ptr:
            pkl.dump(self.data_denom, dst_ptr)
            np.savez(dst_ptr, data = self.data.data ,indices=self.data.indices, indptr =self.data.indptr, shape=self.data.shape)

    @staticmethod
    def deserialize(src):
        with open(src, "rb") as src_ptr:
            data_denom = pkl.load(src_ptr)
            loader = np.load(src_ptr)
            res = scipy.sparse.csr_matrix((loader['data'], loader['indices'], loader['indptr']), shape = loader['shape'])
        return SparseAcoustic(res, data_denom)



def norm(data):
    data_denom = np.sqrt(np.sum(data ** 2))
    data = data/data_denom
    return data, data_denom
