
import collections
import os
import json
import shutil
import re

import run_sim
import multiprocessing

from config import CONST_JSON
from config import RUNS_DIR

def clean_from_comments(f):
    with open(f) as f_ptr:
        f_cont = f_ptr.readlines()
    
    with open(f, 'w') as f_ptr:
        for l in f_cont:
            l_mod = l.split("//")[0].split("#")[0].strip()
            f_ptr.write(l_mod + "\n")


DISTR_RE = "(Norm|Exp)\(([-.0-9]+),[ ]*([-.0-9]+)\)"
distr_re = re.compile(DISTR_RE)

def is_distr(s):
    if distr_re.match(s):
        return True
    return False    

def set_distr_parameter(distr, param_number, value):
    m = distr_re.match(distr)
    if m:
        distr_name = m.group(1)
        if param_number == 0:
            return "%s(%s,%s)" % (distr_name, value, m.group(3))
        elif param_number == 1:
            return "%s(%s,%s)" % (distr_name, m.group(2), value)
        else:
            raise Exception("strange number of parameters: %s" % param_number)
    else:
        raise Exception("failed to parse distr: %s" % distr)

def get_distr_parameter(distr, param_number):
    m = distr_re.match(distr)
    if m:
        return m.group(param_number + 2)
    else:
        raise Exception("failed to parse distr: %s" % distr)



def set_value_in_nested_dict(d, path, value):
    if len(path) > 1:
        if (type(path[0]) is str or type(path[0]) is unicode) and path[0].endswith("distr") and len(path) == 2:
            d[path[0]] = set_distr_parameter(d[path[0]], path[1], value)
        else:
            if not (type(d[path[0]]) is dict or type(d[path[0]]) is collections.OrderedDict or type(d[path[0]]) is list) :
                raise Exception("Unexpected key value: %s" % d[path[0]])
            set_value_in_nested_dict(d[path[0]], path[1:], value)
        return
    d[path[0]] = value

def get_value_in_nested_dict(d, path):
    if len(path) > 1:
        if (type(path[0]) is str or type(path[0]) is unicode) and path[0].endswith("distr") and len(path) == 2:
            return get_distr_parameter(d[path[0]], path[1])
        else:
            if not (type(d[path[0]]) is dict or type(d[path[0]]) is collections.OrderedDict or type(d[path[0]]) is list) :
                raise Exception("Unexpected key value: %s" % d[path[0]])
            return get_value_in_nested_dict(d[path[0]], path[1:])
    return d[path[0]]

def binary_search(a, x, d, lo=0, hi=None, fun = lambda val : val):
    if hi is None:
        hi = len(a)
    while lo < hi:
        mid = (lo+hi)//2
        midval = fun(a[mid])
        diff = midval - x
        if abs(diff) > d:
            if diff<0:
                lo = mid+1
            else:
                hi = mid
        else:
            return mid
    return -1

def is_nested_list(v):
    if type(v) is list and len(v) > 0:
        return type(v[0]) is list
    return False

def evaluate(job_id, params, config, jobs=multiprocessing.cpu_count(), verbose=True):
    run_sim_args = run_sim.RunSimArgs()
    if config.get('input_spikes'):
        run_sim_args.spikes = config['input_spikes']
    elif config.get('input_ts'):
        run_sim_args.input = config['input_ts']
    else:
        raise Exception("Can't find input_spikes or input_ts in config.json")

    run_sim_args.epochs = config['epochs']
    run_sim_args.setEvalCriterion(config['criterion_name'])
    study_dir = os.path.join(config['runs_dir'], "%s_%s" % (config['criterion_name'], config['study']))
    wd = os.path.join(study_dir, str(job_id))
    if not os.path.exists(wd):
        os.makedirs(wd)

    shutil.copy(run_sim_args.const, wd)
    const_json = os.path.join(wd, CONST_JSON)
    run_sim_args.const = const_json
    run_sim_args.working_dir = wd
    run_sim_args.jobs = jobs
    run_sim_args.verbose = verbose
    #run_sim_args.tune_start_weights_to_target_rate = config['tune_start_weights_to_target_rate']

    clean_from_comments(const_json)

    j = json.loads(open(const_json).read(), object_pairs_hook=collections.OrderedDict)
    for k, v in params.items():
        if is_nested_list(config['variables_path'][k]):
            for l in config['variables_path'][k]:
                set_value_in_nested_dict(j, l, v)
        else:                
            set_value_in_nested_dict(j, config['variables_path'][k], v)
    json.dump(j, open(const_json, 'w'), indent=4)
    
    stat = run_sim.main(run_sim_args)
   
    return {
        config['criterion_name'] : stat['eval'],
    }

