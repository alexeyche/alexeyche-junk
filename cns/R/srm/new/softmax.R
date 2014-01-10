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

#              exp(  wk %*% grap_epsp(t, x) )
# rk(t, x) = ---------------------------------
#            sum exp( wl %*% grap_epsp(t, x) )
#             l

# log Pk(y = [tfi] | x ) = sum log( rk(tfi, x) ) -   int rk(t, x) dt  
#                           i                      [0,T] 


# dlog Pk                           drk            1               drk
# ------ (y = [tfi] | x ) =    sum  -- (tfi, x)  -----    -   int  ---(t, x) dt 
# dwk                      i=[1,n]  dwk         rk(tfi, x)  [0,T]  dwk


# drk
# ---(t) = ( 1 - rk(t) ) grab_epsp(t, x)
# dwk
#
# E = SUM(i=1,N){ SUM(k=1,O){ (yk-tk)^2 } } 
# 
# yk = exp(wk*zk)/SUM(j=1,O){ exp(wj*zj) } 
# 
# drk/dwk = zk*rk - zk*yk^2 = zk*yk*(1-yk) % Looks familiar 
# 
# dyk/dwi = -zi*yk^2 * exp(wi*zi-wk*zk) % Don't recall seeing this before 
