
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

cma_conf = {
    "tau_adapt": { "min" : 10, "max" : 500 } ,
    "amp_adapt": { "min" : 0, "max" : 10 },
    "tau_refr": { "min" : 1, "max" : 100 },
    "beta": { "min" : 0.1, "max" : 5.0 },
    "epsp_decay_exc": { "min" : 1, "max" : 100 },
    "epsp_decay_inh": { "min" : 1, "max" : 100 },
    "prob_feedforward_exc" : { "min" : 0.05, "max" : 1.0.0 },
    "prob_feedforward_inh" : { "min" : 0, "max" : 1.0 },
    "prob_reccurent_exc" : { "min" : 0, "max" : 1.0 },
    "prob_reccurent_inh" : { "min" : 0, "max" : 1.0 },
    "weight_distr_mean_ff_exc"  :  { "min" : 0.1, "max" : 100 }, 
    "weight_distr_mean_ff_inh"  :  { "min" : 0, "max" : 100}, 
    "weight_distr_mean_rec_exc" :  { "min" : 0, "max" : 100 }, 
    "weight_distr_mean_rec_inh" :  { "min" : 0, "max" : 100 }, 
}
var_names = sorted(conf['variables_path'])

bounds = [0, 10]

def scale_to_cma(x, min, max, a, b):
    return ((b-a)*(x - min)/(max-min)) + a

def scale_from_cma(x, min, max, a, b):
    return ((max-min)*(x - a)/(b-a)) + min


id = 0
def eval(x):
    global id
    p = dict(zip(var_names, x))
    for param in p:
        p[param] = scale_from_cma(p[param], cma_conf[param]["min"], cma_conf[param]['max'], bounds[0], bounds[1])
    try:
        res = evaluate(id, p, conf)
        ret = res[ conf['criterion_name'] ]
    except KeyboardInterrupt:
        sys.exit(1)
    except:        
        ret = 0.0
    id += 1
    return ret


const = read_const(os.path.join(os.path.dirname(this_file), "../../../const.json"))
start_params = {}
for param in var_names:
    v = float(get_value_in_nested_dict(const, conf['variables_path'][k]))
    start_params[param] = scale_to_cma(v, cma_conf[param]["min"], cma_conf[param]['max'], bounds[0], bounds[1])

es = cma.CMAEvolutionStrategy([ start_params[p] for p in var_names ], 2, { 'bounds' : [0.0,10.0] } )
while not es.stop():
   X = es.ask()
   tells = []
   for x in X:
       print "Launching #%d: " % id
       ret = eval(x)
       tells.append(ret)
   print tells
   es.tell(X, tells)
   es.disp()  # by default sparse, see option verb_disp

cma.pprint(es.result())



