#!/usr/bin/RScript

source('../serialize_to_bin.R')
source('../util.R')

N = 10
p = vector("list",N)
ni = 1
for(i in 0:99) {
    p[[ ni ]] = c(p[[ni]], i)
    ni = ni + 1
    if(ni > N) { 
        ni = 1
    }
}

input_spike_file = "/home/alexeyche/my/sim/input_spikes_test"

saveMatrixList(input_spike_file, list( list_to_matrix(p) ))


stat_f="~/my/sim/stat_file"

p = loadMatrix(stat_f,1)
