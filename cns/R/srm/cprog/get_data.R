#!/usr/bin/RScript

source('../serialize_to_bin.R')
source('../util.R')
source('../plot_funcs.R')

library(snn)

stat_file = "~/my/sim/stat_file"
model_file = "~/my/sim/model_file"

p = loadMatrix(stat_file, 1)
u = loadMatrix(stat_file, 2)
B = loadMatrix(stat_file, 3)
syn1 = loadMatrix(stat_file, 4)
C1 = loadMatrix(stat_file, 5)



W = loadMatrix(model_file,1)
