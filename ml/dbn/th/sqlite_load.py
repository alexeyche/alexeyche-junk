#!/usr/bin/PScript


import sqlite3 as lite
import sys
import numpy as np
import itertools

con = lite.connect('/home/alexeyche/ml.db')

cur = con.cursor()    
if not cur.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='matrices'").fetchall():
    cur.execute("CREATE TABLE matrices(name TEXT, rowNum INT, colNum INT, value REAL)")

def load_np_array(d):
    for k in d:
        arr = d[k]
        t = itertools.chain(*map(lambda y: map(lambda x: (k, x, y, arr[x,y]), xrange(0, arr.shape[0])), xrange(0, arr.shape[1])))
        import pdb; pdb.set_trace()
        cur.executemany("INSERT INTO matrices VALUES(?,?,?,?)",t)
    
    con.commit()
#numpy_rng = np.random.RandomState(1)
#w = np.asarray(0.001 * numpy_rng.randn(5000, 100))



