
import os
import json
import numpy as np
import sys

this_file = os.path.realpath(__file__)
base_dir = os.path.join(os.path.dirname(this_file), "../../")
sys.path.insert(0, base_dir)

from common import evaluate
               
CONFIG=os.path.join(os.path.dirname(this_file), "../snn_config.json")
conf = json.load(open(CONFIG))

conf['variables_path'] = { 
    "tau_adapt": ["neurons", "SRMNeuron", "tau_adapt"],
    "amp_adapt": ["neurons", "SRMNeuron", "amp_adapt"],
    "tau_refr": ["neurons", "SRMNeuron", "tau_refr"],
    "beta": ["act_funcs", "ExpHennequin", "beta"],
    "epsp_decay_exc": ["synapses", "Synapse", "epsp_decay"],
    "epsp_decay_inh": ["synapses", "Synapse_Inh", "epsp_decay"],
    "prob_feedforward_exc" : ["sim_configuration", "conn_map", "0->1", 0, "prob"],
    "prob_feedforward_inh" : ["sim_configuration", "conn_map", "0->1", 1, "prob"],
    "prob_reccurent_exc" : ["sim_configuration", "conn_map", "1->1", 0, "prob"],
    "prob_reccurent_inh" : ["sim_configuration", "conn_map", "1->1", 1, "prob"],
    "weight_distr_mean_ff_exc"  :  ["sim_configuration", "conn_map", "0->1", 0, "weight_distr", 0], 
    "weight_distr_mean_ff_inh"  :  ["sim_configuration", "conn_map", "0->1", 1, "weight_distr", 0], 
    "weight_distr_mean_rec_exc" :  ["sim_configuration", "conn_map", "1->1", 0, "weight_distr", 0], 
    "weight_distr_mean_rec_inh" :  ["sim_configuration", "conn_map", "1->1", 1, "weight_distr", 0], 
}        
conf['study'] = "structure"

def main(job_id, params):
    for p in params:
        params[p] = params[p][0]
    return evaluate(job_id, params, conf, jobs=2)[ conf['criterion_name'] ]
    try:
        return evaluate(job_id, params, conf)
    except Exception as ex:
        print ex
        print 'An error occurred in stdp.py'
        return np.nan
