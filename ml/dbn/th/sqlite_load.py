#!/usr/bin/PScript


import sqlite3 as lite
import sys
import numpy as np
import itertools

con = lite.connect('/home/alexeyche/ml.db')
for t in (np.int8, np.int16, np.int32, np.int64, np.uint8, np.uint16, np.uint32, np.uint64, ):
    lite.register_adapter(t, long)

lite.register_adapter(np.float32, float)

def init_db(con=con):
    cur = con.cursor()    
    if not cur.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='matrices'").fetchall():
        cur.execute("CREATE TABLE matrices(name TEXT, rowNum INT, colNum INT, value REAL, iter INT)")
    con.commit()

init_db()

def flush_db(con=con):
    cur = con.cursor()    
    cur.execute("PRAGMA writable_schema = 1") 
    cur.execute("delete from sqlite_master where type = 'table'")
    cur.execute("PRAGMA writable_schema = 0")
    cur.execute("VACUUM")
    con.commit()
    init_db()

def load_np_array(d, iter, con=con):
    cur = con.cursor()    
    for k in d:
        arr = d[k]
        if len(arr.shape) == 2:
            t = itertools.chain(*map(lambda y: map(lambda x: (k, x, y, arr[x,y], iter), xrange(0, arr.shape[0])), xrange(0, arr.shape[1])))
        if len(arr.shape) == 1:
            t = map(lambda x: (k, x, 0, arr[x], iter), xrange(0, arr.shape[0]))
        cur.executemany("INSERT INTO matrices VALUES(?,?,?,?,?)",t)
    con.commit()


