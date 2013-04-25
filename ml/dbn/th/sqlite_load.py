#!/usr/bin/PScript


import sqlite3 as lite
import sys, time
import numpy as np
import itertools

for t in (np.int8, np.int16, np.int32, np.int64, np.uint8, np.uint16, np.uint32, np.uint64, ):
    lite.register_adapter(t, long)

lite.register_adapter(np.float32, float)

DATABASE = "/home/alexeyche/ml.db"

def init_db():
#    con = lite.connect('/home/alexeyche/ml.db')
    con = lite.connect(DATABASE)
    cur = con.cursor()    
    if not cur.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='matrices'").fetchall():
        cur.execute("CREATE TABLE matrices(name TEXT, rowNum INT, colNum INT, value REAL, iter INT)")
        cur.execute("CREATE INDEX iter_idx ON matrices (iter);")
#        cur.execute("CREATE INDEX row_idx ON matrices (rowNum);")
#        cur.execute("CREATE INDEX col_idx ON matrices (colNum);")
#    cur.execute("PRAGMA journal_mode = OFF")
#    cur.execute("PRAGMA synchronous = OFF")
    cur.execute("PRAGMA read_uncommitted = true")
    con.commit()
    con.close()

init_db()

def flush_db():
    con = lite.connect(DATABASE)
    cur = con.cursor()    
    if cur.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='matrices'").fetchall():
        cur.execute("DELETE FROM matrices")
    con.commit()
    init_db()
    con.close()


def load_np_array(d, iter):
    t = []
    for k in d:
        arr = d[k]
        if len(arr.shape) == 2:
            t += itertools.chain(*map(lambda y: map(lambda x: (k, x, y, arr[x,y], iter), xrange(0, arr.shape[0])), xrange(0, arr.shape[1])))
        if len(arr.shape) == 1:
            t += map(lambda x: (k, x, 0, arr[x], iter), xrange(0, arr.shape[0]))
    con = lite.connect(DATABASE)
    cur = con.cursor()    
    start = time.time()   
    if t:        
        cur.executemany("INSERT INTO matrices VALUES(?,?,?,?,?)",t)
    else:
        print "t is None, arr = %s" % arr
    con.commit()
    con.close()
    end = time.time()
    print "Inserted in %s" % (end - start,)

