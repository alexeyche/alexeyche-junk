#!/usr/bin/env python

import subprocess
import tempfile
import shutil
from numpy.core.defchararray import join as npjoin
import numpy as np
import os
import re
import multiprocessing

jobs = multiprocessing.cpu_count()
snndir = "/home/alexeyche/prog/alexeyche-junk/cns/c"
rundir = "/home/alexeyche/prog/sim/runs"
train_spikes = "/home/alexeyche/prog/sim/spikes/ucr/train_spikes.bin"
calc_spikes = "/home/alexeyche/prog/sim/spikes/ucr/calc_spikes.bin"
epochs = 10
ts_dur = 600

run_sim_cmd = "%(snndir)s/scripts/run_sim.sh -w %(workdir)s -e %(epochs)s -l -a %(train_spikes)s &> %(workdir)s/run_sim.log"
run_sim_calc_cmd = "%(snndir)s/bin/snn_sim -ml %(workdir)s/%(epochs)s_model.bin -l no -sc  %(workdir)s/stat_calc.bin -c %(workdir)s/constants.ini -i %(calc_spikes)s -j %(jobs)s -o %(workdir)s/calc_output_spikes.bin"
calc_cmd = "%(snndir)s/bin/snn_calc -j %(jobs)s -s %(workdir)s/stat_calc.bin -o %(workdir)s/calc.bin -d %(ts_dur)s -p"

def run(d):
    try:
        p = subprocess.Popen(run_sim_cmd % d, stdout=subprocess.PIPE, shell=True) 
        sin, _ = p.communicate()
        print run_sim_calc_cmd % d
        p = subprocess.Popen(run_sim_calc_cmd % d, stdout=subprocess.PIPE, shell=True) 
        sin, _ = p.communicate()
        p = subprocess.Popen(calc_cmd % d, stdout=subprocess.PIPE, shell=True) 
        sin, _ = p.communicate()
        print sin
        return -float(sin.split()[-1].rstrip("\n"))
    except:
        return 100

def np_str(x):
    return [ str(el) for el in x ]


const_fields_to_patch = [ "optimal stdp", "net", "srm neuron" ]

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
                    if current_field in const_fields_to_patch:
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
                  'train_spikes' : train_spikes,
                  'workdir' : workdir,
                  'snndir' : snndir,
                  'calc_spikes' : calc_spikes,
                  'epochs' : epochs,
                  'ts_dur' : ts_dur,
                  'jobs' : jobs,
              }
           )
    return l


if __name__ == '__main__':
    print main(2, { 'net_edge_prob' : [ 0.2 ], })
