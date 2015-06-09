
import os
import json
import numpy as np
import sys
import multiprocessing

this_file = os.path.realpath(__file__)
base_dir = os.path.join(os.path.dirname(this_file), "../")
sys.path.insert(0, base_dir)
base_dir = os.path.join(os.path.dirname(this_file), "../../")
sys.path.insert(0, base_dir)

import cma
from common import evaluate
from common import read_const
from common import get_value_in_nested_dict

CONFIG=os.path.join(os.path.dirname(this_file), "../snn_config.json")
conf = json.load(open(CONFIG))

conf['variables_path'] = { 
    "tau_adapt": ["neurons", "SRMNeuron", "tau_adapt"],
    "amp_adapt": ["neurons", "SRMNeuron", "amp_adapt"],
    "tau_refr": ["neurons", "SRMNeuron", "tau_refr"],
    "beta": ["act_funcs", "ExpHennequin", "beta"],
    "epsp_decay_exc": ["synapses", "SimpleSynapse", "epsp_decay"],
    "epsp_decay_inh": ["synapses", "SimpleSynapse_Inh", "epsp_decay"],
    "prob_feedforward_exc" : ["sim_configuration", "conn_map", "0->1", 0, "prob"],
    "prob_feedforward_inh" : ["sim_configuration", "conn_map", "0->1", 1, "prob"],
    "prob_reccurent_exc" : ["sim_configuration", "conn_map", "1->1", 0, "prob"],
    "prob_reccurent_inh" : ["sim_configuration", "conn_map", "1->1", 1, "prob"],
#    "size" : ["sim_configuration", "net_layers_conf", 0, "size" ],
    "weight_distr_mean_ff_exc"  :  ["sim_configuration", "conn_map", "0->1", 0, "weight_distr", 0], 
    "weight_distr_mean_ff_inh"  :  ["sim_configuration", "conn_map", "0->1", 1, "weight_distr", 0], 
    "weight_distr_mean_rec_exc" :  ["sim_configuration", "conn_map", "1->1", 0, "weight_distr", 0], 
    "weight_distr_mean_rec_inh" :  ["sim_configuration", "conn_map", "1->1", 1, "weight_distr", 0], 
}        
conf['study'] = "structure"

cma_conf = {
    "tau_adapt": { "min" : 10, "max" : 500 } ,
    "amp_adapt": { "min" : 0, "max" : 10 },
    "tau_refr": { "min" : 1, "max" : 50 },
    "beta": { "min" : 0.1, "max" : 7.5 },
    "epsp_decay_exc": { "min" : 1, "max" : 50 },
    "epsp_decay_inh": { "min" : 1, "max" : 50 },
    "prob_feedforward_exc" : { "min" : 0.05, "max" : 1.0 },
    "prob_feedforward_inh" : { "min" : 0, "max" : 1.0 },
    "prob_reccurent_exc" : { "min" : 0, "max" : 0.2 },
    "prob_reccurent_inh" : { "min" : 0, "max" : 1.0 },
#    "size" : { "min" : 2, "max" : 100.0 },
    "weight_distr_mean_ff_exc"  :  { "min" : 0.1, "max" : 100 }, 
    "weight_distr_mean_ff_inh"  :  { "min" : 0, "max" : 100}, 
    "weight_distr_mean_rec_exc" :  { "min" : 0, "max" : 100 }, 
    "weight_distr_mean_rec_inh" :  { "min" : 0, "max" : 100 }, 
}
var_names = sorted(conf['variables_path'])

bounds = [0, 10]
jobs = 2
cma_jobs = 1 # multiprocessing.cpu_count()/jobs

def scale_to_cma(x, min, max, a, b):
    return ((b-a)*(x - min)/(max-min)) + a

def scale_from_cma(x, min, max, a, b):
    return ((max-min)*(x - a)/(b-a)) + min


def eval(x, id, ret_q):
    p = dict(zip(var_names, x))
    for param in p:
        p[param] = scale_from_cma(p[param], cma_conf[param]["min"], cma_conf[param]['max'], bounds[0], bounds[1])
#    p["size"] = int(p["size"])
    try:
        res = evaluate(id, p, conf, jobs, verbose=False)
        ret = res[ conf['criterion_name'] ]
        ret_q.put(ret)
    except Exception as ex:        
        ret_q.put(ex)

const = read_const(os.path.join(os.path.dirname(this_file), "../../../const.json"))
start_params = {}
for param in var_names:
    v = float(get_value_in_nested_dict(const, conf['variables_path'][param]))
    start_params[param] = scale_to_cma(v, cma_conf[param]["min"], cma_conf[param]['max'], bounds[0], bounds[1])

es = cma.CMAEvolutionStrategy([ start_params[p] for p in var_names ], 2, { 'bounds' : [ bounds[0], bounds[1] ], 'popsize' : 20 } )
id = 0
while not es.stop():
    X = es.ask()
    tells = []
    X_work = X
    while X_work:
        pool = []
        for ji in range(cma_jobs):
            if len(X_work) == 0:
                break
            print "Launching #%d: " % id
            ret_q = multiprocessing.Queue()
            #eval(X_work[0], id, ret_q)
            p = multiprocessing.Process(target=eval, args = (X_work[0], id , ret_q))
            X_work = X_work[1:]
            p.start()
            pool.append( (id, p, ret_q) )       
            id += 1
        for id_src, p, ret_q in pool:
            p.join()
            r = ret_q.get()
            if isinstance(r, KeyboardInterrupt):
                raise r
            elif isinstance(r, Exception):
                r = 0.0
            print "got %f from %d" % (r, id_src)
            tells.append(r)
    print tells
    es.tell(X, tells)
    es.disp()  # by default sparse, see option verb_disp

cma.pprint(es.result())



