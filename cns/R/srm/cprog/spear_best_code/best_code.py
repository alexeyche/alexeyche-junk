#!/usr/bin/env python

import subprocess
import tempfile
import shutil
from numpy.core.defchararray import join as npjoin
import numpy as np
import os
import re

snndir = "/home/alexeyche/prog/alexeyche-junk/cns/c"
rundir = "/home/alexeyche/prog/sim/prep_runs"

prep_cmd = "SAMPLE_SIZE=60 WORKDIR=%(workdir)s %(snndir)s/scripts/prep_data.sh"
eval_cmd = "%(snndir)s/bin/snn_postproc -i %(workdir)s/train_spikes.bin -t %(workdir)s/test_spikes.bin -k 5:1:5 -j 4 -o %(workdir)s/output_stat.bin"

def run(d):
    p = subprocess.Popen(prep_cmd % d, stdout=subprocess.PIPE, shell=True) 
    sin, _ = p.communicate()
    p = subprocess.Popen(eval_cmd % d, stdout=subprocess.PIPE, shell=True) 
    sin, _ = p.communicate()
    return 1-float(sin.split()[-1].rstrip("\n"))

def np_str(x):
    return [ str(el) for el in x ]

def patch_and_copy_const(workdir, params):
    def patch_line(l, params):
        for k in params:
            if re.match("^[\s]*%s" % k, l): 
                l_spl = l.split(";")
                if k == "dt":
                    l = "%s = %3.1f" % (k, float(params[k]))
                else:
                    l = "%s = %3.4f" % (k, float(params[k]))
                    
                if len(l_spl) > 1:
                    l += " ; %s" % (l_spl[1], )
        return l
    
    current_field = None
    with open("%s/snn_sim/constants.ini" % snndir) as const_ptr:
        with open("%s/constants.ini" % workdir, 'w') as const_ptr_work:
            for l in const_ptr:
                l = l.strip()
                m = re.match("\[([ A-Za-z]+)\]", l)
                if m:
                    current_field = m.group(1)
                else:
                    if current_field == "adex neuron" or current_field == "preprocess":
                        l = patch_line(l, params)
                    
                const_ptr_work.write(l + "\n")

def main(job_id, params):
    vals = {}
    for p in params:
        vals[p] = np_str(params[p])[0]
    workdir = os.path.join(rundir, str(job_id))
    if not os.path.isdir(workdir):
        os.mkdir(workdir)
    patch_and_copy_const(workdir, vals)

    l = run(  
              { 
                  'snndir' : snndir,
                  'workdir' : workdir,
              }
           )
    return l


if __name__ == '__main__':
    print main(2, { 'Vtr' : [ 11.1 ], 'a' : [ 2 ] })
