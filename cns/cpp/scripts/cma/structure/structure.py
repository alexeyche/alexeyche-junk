
import os
import json
import numpy as np
import sys

this_file = os.path.realpath(__file__)
base_dir = os.path.join(os.path.dirname(this_file), "../")
sys.path.insert(0, base_dir)
base_dir = os.path.join(os.path.dirname(this_file), "../../")
sys.path.insert(0, base_dir)

import cma
from common import evaluate
from run_sim import read_const
from common import get_value_in_nested_dict

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
    return evaluate(job_id, params, conf)

const = read_const(os.path.join(os.path.dirname(this_file), "../../../const.json"))

params = {}
for k in conf['variables_path']:
    params[k] = float(get_value_in_nested_dict(const, conf['variables_path'][k]))

scaling_of_variables = []
cma_conf = {
    "tau_adapt": { "min" : 10, "max" : 500, "scale" : 10 } ,
    "amp_adapt": { "min" : 0, "max" : 50, "scale" : 10 },
    "tau_refr": { "min" : 1, "max" : 100, "scale" : 10 },
    "beta": { "min" : 0.1, "max" : 4.0, "scale" : 10 },
    "epsp_decay_exc": { "min" : 1, "max" : 100, "scale" : 10 },
    "epsp_decay_inh": { "min" : 1, "max" : 100, "scale" : 10 },
    "prob_feedforward_exc" : { "min" : 0.05, "max" : 1.0, "scale" : 1.0 },
    "prob_feedforward_inh" : { "min" : 0, "max" : 1.0, "scale" : 1 },
    "prob_reccurent_exc" : { "min" : 0, "max" : 1.0, "scale" : 1 },
    "prob_reccurent_inh" : { "min" : 0, "max" : 1.0, "scale" : 1 },
    "weight_distr_mean_ff_exc"  :  { "min" : 0.1, "max" : 100, "scale" : 10 }, 
    "weight_distr_mean_ff_inh"  :  { "min" : 0, "max" : 100, "scale" : 10}, 
    "weight_distr_mean_rec_exc" :  { "min" : 0, "max" : 100, "scale" : 10 }, 
    "weight_distr_mean_rec_inh" :  { "min" : 0, "max" : 100, "scale" : 10 }, 
}
var_names = sorted(conf['variables_path'])

es = cma.CMAEvolutionStrategy([ params[p] for p in var_names ], 0.1)
id = 0
while not es.stop():
   X = es.ask()
   print X
   tells = []
   for x in X:
       print x
       import pdb; pdb.set_trace()
       launch_params = dict(zip(var_names, x))
       print "Launching #%d: " % id
       print launch_params
       ret = main(id, launch_params) 
       tells.append( ret[ conf['criterion_name'] ] )
       id += 1
   es.tell(X, tells)
   print "told tells: "
   print tells

   es.disp()  # by default sparse, see option verb_disp

cma.pprint(es.result())



