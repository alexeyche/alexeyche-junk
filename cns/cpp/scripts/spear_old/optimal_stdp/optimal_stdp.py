
import os
import json
import numpy as np
import sys

this_file = os.path.realpath(__file__)
base_dir = os.path.join(os.path.dirname(this_file), "../../")
sys.path.insert(0, base_dir)

from common import evaluate
               
CONFIG="../snn_config.json"               
conf = json.load(open(CONFIG))

conf['variables_path'] = { 
    "weight_decay" :  ["learning_rules", "OptimalStdp"],
    "tau_c" :  ["learning_rules", "OptimalStdp"],
    "beta" : ["act_funcs", "ExpHennequin_Strict"]
}

conf['study'] = "optimal_stdp"

def main(job_id, params):
    try:
        return evaluate(job_id, params, conf)
    except Exception as ex:
        print ex
        print 'An error occurred in unsup_snn.py'
        return np.nan
