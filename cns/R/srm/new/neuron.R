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
                       initialize = function(N, start_weight) {
                         ids <<- get_unique_ids(N)
                         weights <<- list()
                         id_conns <<- list()                         
                         for(i in 1:N) {                           
                           conn <- ids[ ids != ids[i] ] # id of srm neurons: no self connections
                           w <- rep(start_w, N-1)
                           id_conns[[i]] <<- conn
                           weights[[i]] <<- w                           
                         }                         
                         len <<- N
                         #stochastic <<- TRUE
                         weights <<- weights
                         #id_conns <<- id_conns                         
                       },
                       connectFF = function(ids_to_connect, weight) {
                         for(ni in 1:N) {
                           id_conns[[ni]] <<- c(id_conns[[ni]], ids_to_connect)
                           weights[[ni]] <<- c(weights[[ni]], rep(weight, length(ids_to_connect)))
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

