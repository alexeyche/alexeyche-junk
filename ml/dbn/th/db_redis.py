#!/usr/bin/PScript

from redis import Redis
import numpy as np
import struct


r0 = Redis()



form = lambda v : "%3.5f" % v

def load(name, np_arr):
    def load_line(np_1d, key_line):
        v = ' '.join(map(form, np_1d))
        r0.set(key_line, v)

    if len(np_arr.shape) == 2:
        for i in range(np_arr.shape[0]) :
            load_line(np_arr[i,:], key_line = "%s:%d" % (name, i))
    if len(np_arr.shape) == 1:
        load_line(np_arr, key_line = name)
    if len(np_arr.shape) == 0:
        r0.set(name, form(np_arr))

def load_bin(name, np_arr):
    bb = str()
    if len(np_arr.shape) == 2:
        bb = "%i:%i@" % (np_arr.shape[0], np_arr.shape[1])
        for j in xrange(0, np_arr.shape[1]):
            for i in xrange(0, np_arr.shape[0]):
                bb += struct.pack("f", np_arr[i,j])
    if len(np_arr.shape) == 1:
        bb = "%i:1@" % (np_arr.shape[0])
        for i in xrange(0, np_arr.shape[0]):
            bb += struct.pack("f", np_arr[i])
    if len(np_arr.shape) == 0:
        bb = "1:1@" 
        bb += struct.pack("f", np_arr)

    r0.set(name, bb)

def test():
    import numpy as np
    numpy_rng = np.random.RandomState(1)
    arr = np.asarray(numpy_rng.randn(5000,200), dtype=np.float32)
    load_bin("test", arr)
