#!/usr/bin/RScript

source('srm_funcs.R')
source('util.R')

net = list()
net[[1]] <- c(-Inf, 1,2)
net[[2]] <- c(-Inf, 2)
net[[3]] <- c(-Inf, 5)
net[[4]] <- c(-Inf)
net[[5]] <- c(-Inf, 5)

SMNeuronLayer <- setRefClass("SMNeuronLayer", fields = list(w = "list", id_conns = "list", ids = "vector"),
                                              methods = list(
                                              r = function(t, net) {
                                                r_mat = sapply(1:length(w), function(ni) {
                                                    w[[ni]]*sapply(net[ id_conns[[ni]] ], function(sp) sum(epsp(t-sp)))
                                                })
                                                ee = exp(r_mat)
                                                ee/rowSums(ee)
                                              }))


nl = SMNeuronLayer(w = list(c(3,3,3,3,3), c(3,3,3,3,3)), 
              id_conns = list(c(1,2,3,4,5), c(1,2,3,4,5)), 
              ids = c(6,7))

r = sapply(seq(0, 20, by=0.5), function(t) nl$r(t, net))

# exp( sum( epsp(w1, x_i) ) )
# ---------------------------------------------------------
# exp( sum( epsp(w1, x_i) ) ) + exp( sum( epsp(w2, x_i) ) )

