
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
    "tau_plus" :  ["learning_rules", "Stdp", "tau_plus"],
    "tau_minus" :  ["learning_rules", "Stdp", "tau_minus"],
    "a_plus" :  ["learning_rules", "Stdp", "a_plus"],
    "a_minus" :  ["learning_rules", "Stdp", "a_minus"],
    "learning_rate" :  ["learning_rules", "Stdp", "learning_rate"],
    "prob_feedforward" : ["sim_configuration", "conn_map", "0->1", 0, "prob"],
    "prob_reccurent_exc" : ["sim_configuration", "conn_map", "1->1", 0, "prob"],
    "prob_reccurent_inh" : ["sim_configuration", "conn_map", "1->1", 1, "prob"],
}
conf['study'] = "stdp"

def main(job_id, params):
    return evaluate(job_id, params, conf)
    try:
        return evaluate(job_id, params, conf)
    except Exception as ex:
        print ex
        print 'An error occurred in stdp.py'
        return np.nan
