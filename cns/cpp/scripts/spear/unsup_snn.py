import math
import numpy as np
import sys
import os
import re
import shutil
import json
import collections
import multiprocessing

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import run_sim
from config import CONST_JSON
from config import SNN_SIM
from config import SNN_PROC

this_file = os.path.realpath(__file__)

def clean_from_comments(f):
    with open(f) as f_ptr:
        f_cont = f_ptr.readlines()
    
    with open(f, 'w') as f_ptr:
        for l in f_cont:
            l_mod = l.split("//")[0].split("#")[0].strip()
            f_ptr.write(l_mod + "\n")

def set_value_in_nested_dict(d, path, value):
    if len(path) == 1:
        d[path[0]] = value[0]
    else:
        if not type(d[path[0]]) is collections.OrderedDict and not type(d[path[0]]) is dict:
            raise Exception("Unexpected key value: %s" % d[path[0]])
        set_value_in_nested_dict(d[path[0]], path[1:], value)

                
EPOCHS_NUM = 10
INPUT_DATA = "/home/alexeyche/prog/sim/synth_control.512.6_by_10.pb"
RUNS_DIR = "/home/alexeyche/prog/sim_spear"

variables_path = { 
    'weight_decay' :  ["learning_rules", "OptimalStdp"],
    'tau_c' :  ["learning_rules", "OptimalStdp"],
    'beta' : ["act_funcs", "ExpHennequin_Strict"],
}

def evaluate(job_id, params):
    wd = os.path.join(RUNS_DIR, str(job_id))
    if not os.path.exists(wd):
        os.makedirs(wd)
    shutil.copy(os.path.join(os.path.dirname(this_file), "../../", CONST_JSON), wd)
    
    const_json = os.path.join(wd, CONST_JSON)
    clean_from_comments(const_json)

    j = json.loads(open(const_json).read(), object_pairs_hook=collections.OrderedDict)
    for k, v in params.items():
        set_value_in_nested_dict(j, variables_path[k] + [k], v)
    json.dump(j, open(const_json, 'w'), indent=4)
    
    run_sim_args = run_sim.RunSimArgs()
    run_sim_args.input = INPUT_DATA
    run_sim_args.epochs = EPOCHS_NUM
    run_sim_args.working_dir = wd
    run_sim_args.const = const_json
    
    stat = run_sim.main(run_sim_args)
   
    return {
        "cluster_criterion" : stat['eval'],
#        "mean_rate" : stat['mean_rate'],
    }

def main(job_id, params):
    try:
        return evaluate(job_id, params)
    except Exception as ex:
        print ex
        print 'An error occurred in unsup_snn.py'
        return np.nan
