
import collections
import os
import json
import shutil
import re
import traceback
import sys

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

def read_const(const_file):
    with open(const_file) as const_ptr:
        c_json = "\n".join([ l.split("//")[0].split("#")[0] for l in const_ptr.readlines() ])
    try:
        const = json.loads(c_json, object_pairs_hook=collections.OrderedDict)
    except:
        print "Error while reading %s:" % const_file
        print traceback.format_exc()
        sys.exit(1) 
    return const



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
    run_sim_args = run_sim.SnnSim.Args()
    if config.get('input_spikes'):
        run_sim_args.spikes = config['input_spikes']
    elif config.get('input_ts'):
        run_sim_args.input = config['input_ts']
    else:
        raise Exception("Can't find input_spikes or input_ts in config.json")
    if config.get('test_spikes'):
        run_sim_args.test_spikes = config['input_spikes']

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


DISTR_REG_EXP = "[A-Za-z]+\(([ 0-9.]+),([ 0-9.]+)\)"

def tuneStartWeights(args, wd, common_sim_args, weight_range=(2.5, 100, 500), T_max=2500, parallel_launches=4):
    const = read_const(common_sim_args['--constants'])
    d_re = re.compile(DISTR_REG_EXP)
    acc_w = 0.0
    conn_count = 0
    for c in const['sim_configuration']['conn_map']:
        for c_var in const['sim_configuration']['conn_map'][c]:
            m = d_re.match(c_var['weight_distr'])
            if m:
                acc_w += float(m.group(1))
                conn_count += 1
            else:
                raise Exception("Faild to parse distribution of weighgts: %s" % c_var['weight_distr'])
    start_weight = acc_w / conn_count

    sim_args = common_sim_args.copy()
    sim_args['--no-learning'] = True
    sim_args['--T-max'] = T_max
    sim_args['--constants'] = os.path.join(wd, "tune_const.json")
    
    def run_one_tune(args, wd, sim_args, const, w):
        for c in const['sim_configuration']['conn_map']:
            for i, c_var in enumerate(const['sim_configuration']['conn_map'][c]):
                c_var['weight_distr'] = "Norm(%s,0.5)" % w
                const['sim_configuration']['conn_map'][c][i] = c_var
        json.dump(const, open(sim_args['--constants'], 'w'), indent=4)

        procs = []
        for i in range(parallel_launches):
            sim_args_launch = sim_args.copy()
            sim_args_launch['--jobs'] = int(math.floor(float(sim_args_launch['--jobs'])/parallel_launches))
            if i == parallel_launches-1:
                 sim_args_launch['--jobs']  += sim_args['--jobs'] - sim_args_launch['--jobs'] * parallel_launches
            sim_args_launch['--output'] = os.path.join(wd, "tune_weights_output_spikes_%s.pb" % i)
            sp = runProcess(args.snn_sim_bin, sim_args_launch, os.path.join(wd, "tune_weights_output_%s.log" % i), verbose = args.verbose, do_not_wait=True)
            procs.append( (sim_args_launch, sp) )
        
        mean_acc = 0.0
        for sim_args_launch, sp in procs:
            try:
                stdout, stderr = sp.communicate()
            except KeyboardInterrupt:
                print "Bye"
            if sp.returncode != 0:
                print "Run failed: "
                for l in stdout.split("\n"):
                    print l.strip()
                sys.exit(1)
            proc_args = {
                '--spikes' : sim_args_launch['--output'],
                '--net-neurons' : sum([ conf['size'] for conf in  const['sim_configuration']['net_layers_conf'] ]),
            }
            json_output = runProcess([args.snn_proc_bin, "mean_net_rate"], proc_args, verbose=args.verbose, json_stdout=True)
            mean_acc += json_output['mean_rate']

        mean = mean_acc/parallel_launches            
        print "weight %s got %s mean rate" % (w, mean)
        return mean
    

    weights = np.linspace(weight_range[0], weight_range[1], weight_range[2])
    part_fun_tune = functools.partial(run_one_tune, args, wd, sim_args, const)

    pos = common.binary_search(weights, args.tune_start_weights_to_target_rate, 0.5, fun=part_fun_tune)
    if pos < 0:
        raise Exception("Failed to find good weight")

    new_const = read_const(sim_args['--constants'])
    for c in new_const['sim_configuration']['net_layers_conf']:
        #if 'learning_rule' in c['neuron_conf']:
        #    new_const['learning_rules'][ c['neuron_conf']['learning_rule'] ]['learning_rate'] *= weights[pos]/start_weight
        if 'weight_normalization' in c['neuron_conf']:
            if 'w_max' in new_const['weight_normalizations'][ c['neuron_conf']['weight_normalization'] ]:
                new_const['weight_normalizations'][ c['neuron_conf']['weight_normalization'] ]['w_max'] *= weights[pos]/start_weight
    json.dump(new_const, open(sim_args['--constants'], 'w'), indent=4)
    common_sim_args['--constants'] = sim_args['--constants']

   
