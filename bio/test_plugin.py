import gc
import numpy as np
from util import shl, shm, shs
from functools import partial
from activation import *
from datasets import get_toy_data, quantize_data
import time
import weave
from sklearn.metrics import log_loss        


import numpy
import ctypes

# pointer to float type, for convenience
c_float_p = ctypes.POINTER(ctypes.c_float)

_net_step = ctypes.cdll.LoadLibrary("/Users/aleksei/distr/alexeyche-junk/bio/net_step.so")
_net_step.net_step.argtypes = (c_float_p, ctypes.c_uint32, ctypes.c_uint32)
net_step = _net_step.net_step

v = np.ascontiguousarray(np.random.randn(2, 5), dtype=np.float32)

# v = np.asarray([[1.0, 2.0], [3.0, 4.0]], dtype=np.float32)


net_step(v.ctypes.data_as(c_float_p), v.shape[0], v.shape[1])



# def f():
# 	v = 0.0
# 	for i in xrange(100):
# 		v += _net_step.net_step(i)
# 	return v

# import cProfile
# pf = '/Users/aleksei/tmp/profile'
# cProfile.run('_net_step.net_step(5.0)', pf)

# import pstats
# p = pstats.Stats(pf)
# p.strip_dirs().sort_stats("cumtime").print_stats()    

