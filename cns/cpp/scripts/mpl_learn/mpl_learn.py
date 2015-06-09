import os
import sys
import logging
from os.path import join as pj
from os.path import expanduser as eu
import numpy as np
import multiprocessing
import subprocess as sub
from contextlib import contextmanager
import shutil
import json

import cma
from gammatone import gammatone
from dnn_protos import writeDoubleMatrix
from color_log import add_coloring_to_emit_ansi

POP_SIZE = 50
SIGMA = 2
BOUNDS = (0, 10)
CMA_JOBS = 4
VERBOSE = True

RUNS_DIR = eu("~/dnn/mpl/cma_runs")
MPL_BIN = eu("~/cpp/build/bin/mpl")
MPL_CONF_JSON = eu("~/cpp/mpl.json")
MPL_TEST_DATA = eu("~/dnn/ts/riken_14chan_3LRR.pb")
MPL_DIM = 0

cma_conf = {
    "log_freq": { "def" : True },    # non learn
    "a" : { "def" : 10.0, },         # non learn
    "b" : { "def" : 0.1, "min" : 0.1, "max" : 10},
    "n": { "def" : 6, "min" : 0.1, "max" : 10},
    "fnum": { "def" : 24}, #"min" : 12, "max" : 24},
    "lb": { "def" : 0.0, "min" : 0, "max" : 5},
    "hb": { "def" : 50.0, "min" : 10, "max" : 300},
    "L": { "def" : 100.0}, # "min" : 10, "max" : 100},
    "Tmax": { "def" : 1.0, "min" : 0.1, "max" : 10},
    "phi": { "def" : 0.0, "min" : 0.0, "max" : 2*np.pi},
}
cma_conf_learn = dict([ (k, v) for k, v in cma_conf.iteritems() if not v.get("min") is None and not v.get("max") is None ])
cma_conf_consts = dict([ (k, v["def"]) for k, v in cma_conf.iteritems() if not k in cma_conf_learn ])
var_names = sorted(cma_conf_learn)

@contextmanager
def pushd(newDir):
    previousDir = os.getcwd()
    os.chdir(newDir)
    yield
    os.chdir(previousDir)

def scale_to_cma(x, min, max, a, b):
    return ((b-a)*(x - min)/(max-min)) + a

def scale_from_cma(x, min, max, a, b):
    return ((max-min)*(x - a)/(b-a)) + min

logFormatter = logging.Formatter("%(asctime)s [%(threadName)s] [%(levelname)s]  %(message)-100s")
rootLogger = logging.getLogger()

consoleHandler = logging.StreamHandler(sys.stdout)
consoleHandler.emit = add_coloring_to_emit_ansi(consoleHandler.emit)
consoleHandler.setFormatter(logFormatter)

if VERBOSE:
    rootLogger.setLevel(logging.DEBUG)
else:
    rootLogger.setLevel(logging.INFO)
rootLogger.addHandler(consoleHandler)

def eval(x, id, ret_q):
    p = dict(zip(var_names, x))
    for param in p:
        p[param] = scale_from_cma(p[param], cma_conf[param]["min"], cma_conf[param]['max'], BOUNDS[0], BOUNDS[1])
    p.update(cma_conf_consts)

    p["fnum"] = int(round(p["fnum"]))
    p["L"] = int(round(p["L"]))
    

    f = gammatone(**p)
    rd = pj(RUNS_DIR, str(id))
    os.makedirs(rd)

    mpl_conf = json.load(open(MPL_CONF_JSON))
    mpl_conf[ mpl_conf.keys()[0] ]["filters_num"] = p["fnum"]
    mpl_conf[ mpl_conf.keys()[0] ]["filter_size"] = p["L"]
    mpl_file = pj(rd, os.path.basename(MPL_CONF_JSON))
    with open(mpl_file, "w") as fptr:
        json.dump(mpl_conf, fptr, indent=2)
    
    filt_file = pj(rd, "filter.pb")
    writeDoubleMatrix(f, filt_file)
    rest_file = pj(rd, "restored.pb")
    with pushd(rd):
        cmd = [MPL_BIN, "-c", mpl_file, "-i", MPL_TEST_DATA, "-f", filt_file, "-d", str(MPL_DIM), "-r", rest_file]
        logging.info("Running {}".format(" ".join(cmd)))
        proc = sub.Popen(cmd, stdout=sub.PIPE, stderr=sub.PIPE)
        stdout, stderr = proc.communicate()
        if proc.returncode != 0:
            raise Exception("Found not null ret code:\n{}\n{}".format(stdout, stderr))
        logging.debug("=============")
        logging.debug(stdout)
        logging.debug("=============")
        logging.debug(stderr)
    stdout = stdout.split("\n")
    
    while not stdout[-1]:
        stdout = stdout[:-1]
    _, err = stdout[-1].split(":", 2)
    ret_q.put(float(err))


start_params = dict([ (k, scale_to_cma(v["def"], v["min"], v["max"], BOUNDS[0], BOUNDS[1])) for k, v in cma_conf_learn.iteritems() ])

runs = os.listdir(RUNS_DIR)
for r in runs:
    fr = pj(RUNS_DIR, r)
    logging.warning("Cleaning {} ... ".format(fr))
    shutil.rmtree(fr)

es = cma.CMAEvolutionStrategy([ start_params[p] for p in var_names ], SIGMA, { 'bounds' : [ BOUNDS[0], BOUNDS[1] ], 'popsize' : POP_SIZE } )
id = 0
while not es.stop():
    X = es.ask()
    tells = []
    X_work = X
    while X_work:
        pool = []
        for ji in range(CMA_JOBS):
            if len(X_work) == 0:
                break
            ret_q = multiprocessing.Queue()
            #eval(X_work[0], id, ret_q)
            p = multiprocessing.Process(target=eval, args = (X_work[0], id , ret_q))
            p.start()
            X_work = X_work[1:]
            pool.append( (id, p, ret_q) )       
            id += 1
        for id_src, p, ret_q in pool:
            p.join()
            r = ret_q.get()
            if isinstance(r, KeyboardInterrupt):
                raise r
            elif isinstance(r, Exception):
                r = 0.0
            logging.info("got %f from %d" % (r, id_src))
            tells.append(r)
    logging.info("Got population: {}".format(tells))
    es.tell(X, tells)
    es.disp()  # by default sparse, see option verb_disp

ans = dict([ (param, scale_from_cma(es.result()[0][i], cma_conf[param]["min"], cma_conf[param]['max'], BOUNDS[0], BOUNDS[1])) for i, param in enumerate(var_names) ])
for k, v in ans.iteritems():
    logging.info("Got optimal values: {}: {}".format(k, v))
