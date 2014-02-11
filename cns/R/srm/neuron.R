#!/usr/bin/RScript

source('srm_funcs.R')
source('llh.R')
require(snn)

neuron = setRefClass("neuron", fields = list(w = "vector", id_conn = "vector", id = "vector"),
                                methods = list( 
                                u = function(t, net) {
                                    USRM(t, constants, id, id_conn, w, net)
                                    #e_syn= sapply(net[id_conn], function(sp) sum(epsp(t-sp)))
                                    #u_rest + sum(w*e_syn) + sum(nu(t-net[[id]]))
                                },
                                p = function(t, net) {
                                    g(u(t,net))
                                },
                                get_id = function() {
                                  id
                                }))


SRMLayer = setRefClass("SRMLayer", fields = list(weights = "list", id_conns = "list", ids = "vector", len="vector", stochastic="logical", 
                                                 mean_act="vector", epsps_cur="list",  Cacc="list", Bacc="list", current_u="vector"),
                     methods = list(                       
                       initialize = function(N, start_weight, p_edge_prob=1, ninh=0) {
                         ids <<- get_unique_ids(N)
                         weights <<- list()
                         id_conns <<- list()                         
                         if( is.matrix(start_weight) ) {
                           start_w = start_weight
                         } else {
                           start_w = matrix(start_weight, ncol=N, nrow=N-1)
                         }
                         if(ninh>0) {
                            inh_idxs = ids[(length(ids)-ninh+1):length(ids)]
                         } else {
                            inh_idxs = NULL
                         } 
                         for(i in 1:N) {                                                      
                           full_conn <- ids[ ids != ids[i] ] # id of srm neurons: no self connections
                           conn_exists = p_edge_prob > runif(length(full_conn))
                           w <- start_w[,i]
                           id_conns[[i]] <<- full_conn[conn_exists]
                           weights[[i]] <<- w[conn_exists]          
                           inh = id_conns[[i]] %in% inh_idxs
                           if(any(inh))
                              weights[[i]][inh]  <<- -weights[[i]][inh]
                         }
                         
                         len <<- N
                         #stochastic <<- TRUE
                         weights <<- weights                         
                         mean_act <<- rep(0, N)
                         
                         Cacc <<- lapply(1:N, function(ni) rep(0, length(id_conns[[ni]])))
                         Bacc <<- lapply(1:N, function(ni) rep(0, length(id_conns[[ni]])))
                         current_u <<- rep(0, N)
                         #id_conns <<- id_conns       
                         mean_act <<- rep(0, N)
                       },
                       connectFF = function(ids_to_connect, weight, neurons_to_connect=NULL) {
                         if(is.null(neurons_to_connect)) {
                           neurons_to_connect=1:.self$len
                         }
                         if(!is.matrix(ids_to_connect)) {
                           ids_to_connect = matrix(ids_to_connect, nrow=length(ids_to_connect), ncol=length(neurons_to_connect))
                         }                         
                         
                         if(!is.matrix(weight)) {
                           weight = matrix(weight, nrow=nrow(ids_to_connect), ncol=length(neurons_to_connect))
                         }
                         
                         for(ni in neurons_to_connect) {
                           conn_exists = ids_to_connect[,ni] != 0
                           id_conns[[ni]] <<- c(id_conns[[ni]], ids_to_connect[conn_exists,ni])                           
                           weights[[ni]] <<- c(weights[[ni]], weight[conn_exists,ni])
                         }
                         Cacc <<- lapply(1:.self$len, function(ni) rep(0, length(id_conns[[ni]])))
                         Bacc <<- lapply(1:.self$len, function(ni) rep(0, length(id_conns[[ni]])))
                       },
                       u = function(time, net) {
                         c(u, epsps_cur) := USRMsFull(time, constants, .self$ids, .self$id_conns, .self$weights, net)                         
                         .self$epsps_cur <- epsps_cur                         
                         .self$current_u <- u
                         .self$mean_act <- rep(1, .self$len) #.self$mean_act - .self$mean_act/1000 + probf(u)
                         return(u)
                       },
                       uFull = function(time, net) {
                         USRMsFull(time, constants, .self$ids, .self$id_conns, .self$weights, net)
                       },
                       C = function(t, dt, net) {
                         Y = sp_in_interval(net[.self$ids], t-dt, t)                                                  
                         C_curr = lapply(1:.self$len, function(ni) {
                           if(!is.null(Y[[ni]])) {
                             .self$epsps_cur[[ni]]/probf(.self$current_u[ni]) - .self$epsps_cur[[ni]]
                           } else {
                             0 - .self$epsps_cur[[ni]]
                           }
                         })
                         .self$Cacc <- lapply(1:.self$len, function(ni) .self$Cacc[[ni]]*0.99 + C_curr[[ni]])
                         return(.self$Cacc)
                       },
                       B = function(t, dt, net) {
                         Y = sp_in_interval(net[.self$ids], t-dt, t)                                                  
                         B_curr = lapply(1:.self$len, function(ni) {
                           if(!is.null(Y[[ni]])) {
                             probf(.self$current_u[ni])/.self$mean_act[ni] - (probf(.self$current_u[ni]) - .self$mean_act[ni])
                           } else {
                             0 - (probf(.self$current_u[ni]) - .self$mean_act[ni])
                           }
                         })
                         .self$Bacc <- lapply(1:.self$len, function(ni) B_curr[[ni]])
                         return(.self$Bacc)
                       },
                       u_one = function(num, t, net) {
                         USRM(t, constants, ids[num], id_conns[[num]], weights[[num]], net)
                       },
                       get = function(id) {
                         return(list(w=weights[[id]],id_conn=id_conns[[id]],id=ids[[id]]))
                       },
                       grad = function(T0, Tmax, net, target_set) {
                         return(grad_func(.self, T0, Tmax, net))
                       },
                       P = function(T0, Tmax, net) {
                         pnf = probNoFire(T0, Tmax, .self, net, constants)$out
                         spikes = sp_in_interval(net[.self$ids], T0, Tmax)
                         sapply(1:.self$len, function(ni) {
                           p = pnf[ni]
                           if(!is.null(spikes[[ni]])) {
                             p = p*Reduce("*", probf(spikes[[ni]]))
                           }
                           return(p)                           
                         })
                       },
                       epsp_fun_one = function(t, net, ni) {
                         neuron_epsp(t, constants, .self$ids[ni], .self$id_conns[[ni]], .self$weights[[ni]], net)$out
                       },
                       to_list = function() {
                         neurons = list()
                         for(i in 1:.self$len) {
                           neurons[[i]] = neuron(w=weights[[i]], id_conn=id_conns[[i]], id=ids[i])
                         }
                         return(neurons)
                       }
                       ))

SimLayers = setRefClass("SimLayers", fields=list(l="list", all_ids="vector"), methods=list(
                    initialize = function(layers_c) {
                        all_ids <<- unlist(lapply(layers_c, function(n) n$ids))
                        l <<- layers_c
                    },
                    sim = function(sim_options, net) {
                      simLayers(sim_options, constants, .self$l, net)
                      for(n in .self$l) {
                         mean_act = sapply(net[n$ids], length)/(sim_options$Tmax-sim_options$T0)
                      }
                    }))

test_neurons = function() {
  net <- list()
  net[[1]] <- c(10, 11, 12)
  net[[2]] <- c(5, 5, 5)
  net[[3]] <- c(1, 2, 3)
  
  n = SRMLayer( weights = list( c(1,1,1), c(1,1,1), c(1,1,1)), id_conns = list( c(2,3), c(1,3), c(1,2)), ids = c(1,2,3))
  uu = n$u(seq(0,20, by=0.5), net)
  plot(uu[1,], type="l")
  lines(uu[2,])
  lines(uu[3,])
}

test = function() {
    net <- list()
    net[[1]] <- c(10, 11, 12)
    net[[2]] <- c(5, 5, 5)
    net[[3]] <- c(1, 2, 3)


    n1 = neuron(w = c(1, 1, 1), id_conn = c(2,3), id = 1)
    n2 = neuron(w = c(1, 1, 1), id_conn = c(1,2), id = 2)
    n3 = neuron(w = c(1, 1, 1), id_conn = c(1,3), id = 3)

    uu <- c()
    for(t in seq(1, 100)) {
        uu <- c(uu, n1$u(t, net))
    }
    plot(uu, type="l")
}

