#    This file is part of DEAP.
#
#    DEAP is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as
#    published by the Free Software Foundation, either version 3 of
#    the License, or (at your option) any later version.
#
#    DEAP is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with DEAP. If not, see <http://www.gnu.org/licenses/>.

import os
import json
import numpy as np
import sys
import math

this_file = os.path.realpath(__file__)
base_dir = os.path.join(os.path.dirname(this_file), "../")
sys.path.insert(0, base_dir)
base_dir = os.path.join(os.path.dirname(this_file), "../../")
sys.path.insert(0, base_dir)

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
    "tau_adapt": { "min" : 10, "max" : 300, "scale" : 10 } ,
    "amp_adapt": { "min" : 0, "max" : 10, "scale" : 10 },
    "tau_refr": { "min" : 1, "max" : 20, "scale" : 10 },
    "beta": { "min" : 0.1, "max" : 4.0, "scale" : 10 },
    "epsp_decay_exc": { "min" : 1, "max" : 100, "scale" : 10 },
    "epsp_decay_inh": { "min" : 1, "max" : 100, "scale" : 10 },
    "prob_feedforward_exc" : { "min" : 0.05, "max" : 1.0, "scale" : 1.0 },
    "prob_feedforward_inh" : { "min" : 0, "max" : 1.0, "scale" : 1 },
    "prob_reccurent_exc" : { "min" : 0, "max" : 1.0, "scale" : 1 },
    "prob_reccurent_inh" : { "min" : 0, "max" : 1.0, "scale" : 1 },
    "weight_distr_mean_ff_exc"  :  { "min" : 0.1, "max" : 200, "scale" : 10 }, 
    "weight_distr_mean_ff_inh"  :  { "min" : 0, "max" : 200, "scale" : 10}, 
    "weight_distr_mean_rec_exc" :  { "min" : 0, "max" : 200, "scale" : 10 }, 
    "weight_distr_mean_rec_inh" :  { "min" : 0, "max" : 200, "scale" : 10 }, 
}
var_names = sorted(conf['variables_path'])

def base_scale(x, a, b, p = 1):
    return a + (b-a)*( (x-(-7.5))/15.0)**p

def scale(p):
    for param in p:
        p[param] = base_scale(p[param], cma_conf[param]["min"], cma_conf[param]['max'])

id = 0
def eval(x):
    global id
    p = dict(zip(var_names, x))
    scale(p)
    print p
    try:
        res = main(id, p)
        ret = [ res[ conf['criterion_name'] ] ]
    except:
        ret = 0.0
    id += 1
    return ret

import array
import numpy

from deap import algorithms
from deap import base
from deap import benchmarks
from deap import cma
from deap import creator
from deap import tools

N=len(conf['variables_path'])
creator.create("FitnessMin", base.Fitness, weights=(1.0,))
creator.create("Individual", array.array, typecode='d', fitness=creator.FitnessMin)

# See http://www.lri.fr/~hansen/cmaes_inmatlab.html for more details about
# the rastrigin and other tests for CMA-ES
toolbox = base.Toolbox()
toolbox.register("evaluate", eval)

def main_entry():
    numpy.random.seed()

    # The CMA-ES One Plus Lambda algorithm takes a initialized parent as argument
    parent = creator.Individual([ params[v] for v in var_names ])
    parent.fitness.values = toolbox.evaluate(parent)
    
    strategy = cma.StrategyOnePlusLambda(parent, sigma=1.5, lambda_=10)
    toolbox.register("generate", strategy.generate, ind_init=creator.Individual)
    toolbox.register("update", strategy.update)

    hof = tools.HallOfFame(1)    
    stats = tools.Statistics(lambda ind: ind.fitness.values)
    stats.register("avg", numpy.mean)
    stats.register("std", numpy.std)
    stats.register("min", numpy.min)
    stats.register("max", numpy.max)
   
    algorithms.eaGenerateUpdate(toolbox, ngen=1000, halloffame=hof, stats=stats)

if __name__ == "__main__":
    main_entry()
