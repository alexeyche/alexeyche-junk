#!/usr/bin/env python

import subprocess
import tempfile
import shutil
from numpy.core.defchararray import join as npjoin
import numpy as np

#codedir = '/home/alexeyche/my/git/alexeyche-junk/cns/R/srm/tem'
codedir = "/home/alexeyche/prog/alexeyche-junk/cns/R/srm/tem"

script_name = 'find_best_code.R'
cmd = "Rscript %(codedir)s/%(script_name)s --params=\"%(params)s\""

def run(d):
    p = subprocess.Popen(cmd % d, stdout=subprocess.PIPE, shell=True) 
    sin, _ = p.communicate()
    return float(sin.split()[-1].strip())

def np_str(x):
    return [ str(el) for el in x ]


def main(job_id, params):
    p = []
    p.append(" ".join(np_str(params["bias"])))
    p.append(" ".join(np_str(params["thresh"])))
    p.append(" ".join(np_str(params["capacitance"])))
    p.append(" ".join(np_str(params["resistance"])))
    l = run(  
              { 
                  'codedir' : codedir, 
                  'script_name' : script_name,
                  'params' : "|".join(p),
              }
           )
    return l


if __name__ == '__main__':
    p = []
    p.append(" ".join([ str(el) for el in np.asarray([1,2,3])]))
    print p
