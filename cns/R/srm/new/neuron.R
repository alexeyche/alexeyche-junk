#!/usr/bin/RScript

source('srm_funcs.R')

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
