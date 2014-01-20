#!/usr/bin/RScript

source('srm_funcs.R')
source('llh.R')
require(snnSRM)
constants = list(e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest)

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


SRMLayer = setRefClass("SRMLayer", fields = list(weights = "list", id_conns = "list", ids = "vector", len="vector", stochastic="logical"),
                     methods = list(                       
                       initialize = function(N, start_weight, p_edge_prob=1) {
                         ids <<- get_unique_ids(N)
                         weights <<- list()
                         id_conns <<- list()                         
                         if( is.matrix(start_weight) ) {
                           start_w = start_weight
                         } else {
                           start_w = matrix(start_weight, ncol=N, nrow=N-1)
                         }
                         for(i in 1:N) {                           
                           full_conn <- ids[ ids != ids[i] ] # id of srm neurons: no self connections
                           conn_exists = p_edge_prob > runif(length(full_conn))
                           w <- start_w[,i]
                           id_conns[[i]] <<- full_conn[conn_exists]
                           weights[[i]] <<- w[conn_exists]                           
                         }                         
                         len <<- N
                         #stochastic <<- TRUE
                         weights <<- weights
                         #id_conns <<- id_conns                         
                       },
                       connectFF = function(ids_to_connect, weight, neurons_to_connect=NULL) {
                         if(is.null(neurons_to_connect)) {
                           neurons_to_connect=1:.self$len
                         }
                         if(!is.matrix(weight)) {
                           weight = matrix(weight, nrow=nrow(ids_to_connect), ncol=length(neurons_to_connect))
                         }
                         if(!is.matrix(ids_to_connect)) {
                            ids_to_connect = matrix(ids_to_connect, nrow=nrow(ids_to_connect), ncol=length(neurons_to_connect))
                         }                         
                         
                         for(ni in neurons_to_connect) {
                           conn_exists = ids_to_connect[,ni] != 0
                           id_conns[[ni]] <<- c(id_conns[[ni]], ids_to_connect[conn_exists,ni])                           
                           weights[[ni]] <<- c(weights[[ni]], weight[conn_exists,ni])
                         }
                       },
                       u = function(time, net) {
                         USRMs(time, constants, .self$ids, .self$id_conns, .self$weights, net)
                         #e_syn= sapply(net[id_conn], function(sp) sum(epsp(t-sp)))
                         #u_rest + sum(w*e_syn) + sum(nu(t-net[[id]]))
                       },
                       u_one = function(num, t, net) {
                         USRM(t, constants, ids[[num]], id_conns[[num]], weights[[num]], net)
                         #e_syn= sapply(net[id_conn], function(sp) sum(epsp(t-sp)))
                         #u_rest + sum(w*e_syn) + sum(nu(t-net[[id]]))
                       },

                       get = function(id) {
                         return(list(w=weights[[id]],id_conn=id_conns[[id]],id=ids[[id]]))
                       },
                       grad = function(T0, Tmax, net, target_set) {
                         return(grad_func(.self, T0, Tmax, net, target_set))
                       }, 
                       to_list = function() {
                         neurons = list()
                         for(i in 1:.self$len) {
                           neurons[[i]] = neuron(w=weights[[i]], id_conn=id_conns[[i]], id=ids[i])
                         }
                         return(neurons)
                       }
                       ))

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

