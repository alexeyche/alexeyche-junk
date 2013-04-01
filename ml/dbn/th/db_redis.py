#!/usr/bin/PScript

from redis import Redis
import numpy as np
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

