#!/usr/bin/RScript


SRMLayerClass = setRefClass("SRMLayerClass", fields = c("obj", "prop"),
                     methods = list(                       
                       initialize = function(N, start_weight, p_edge_prob, ninh=0) {
                         obj <<- new(SRMLayer, N)
                         prop <<- list(edge_prob=p_edge_prob, ninh=ninh)
                         ids <- get_unique_ids(N)
                         obj$W <<- list()
                         obj$id_conns <<- list()                         
                         obj$syn <<- list()
                         obj$syn_spec <<- list()
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
                           obj$id_conns[[i]] <<- full_conn[conn_exists]
                           obj$W[[i]] <<- w[conn_exists]          
                           inh = obj$id_conns[[i]] %in% inh_idxs
                           obj$syn[[i]] <<- rep(0, length(obj$W[[i]]))
                           obj$syn_spec[[i]] <<- rep(1, length(obj$W[[i]]))
                           if(any(inh)) {
                               obj$syn_spec[[i]][inh] <<- rep(-1, length(obj$syn_spec[[i]][inh]))
                           }
                         }
                         
                         obj$setNum(N)
                         obj$ids <<- ids
                         obj$C <<- obj$syn
                       },
                       connectFF = function(ids_to_connect, weight, neurons_to_connect=NULL) {
                         if(is.null(neurons_to_connect)) {
                           neurons_to_connect=1:obj$num()
                         }
                         if(!is.matrix(ids_to_connect)) {
                           ids_to_connect = matrix(ids_to_connect, nrow=length(ids_to_connect), ncol=length(neurons_to_connect))
                         }                         
                         
                         if(!is.matrix(weight)) {
                           weight = matrix(weight, nrow=nrow(ids_to_connect), ncol=length(neurons_to_connect))
                         }
                         
                         for(ni in neurons_to_connect) {
                           conn_exists = ids_to_connect[,ni] != 0
                           obj$id_conns[[ni]] <<- c(obj$id_conns[[ni]], ids_to_connect[conn_exists,ni])                           
                           obj$W[[ni]] <<- c(obj$W[[ni]], weight[conn_exists,ni])
                           obj$syn[[ni]] <<- rep(0, length(obj$W[[ni]]))
                           obj$syn_spec[[ni]] <<- rep(1, length(obj$W[[ni]]))
                         }
                         obj$C <<- obj$syn
                       },
                       ids = function() {
                        return (.self$obj$ids)
                       },
                       W = function() {
                        return (.self$obj$W)
                       },
                       Wm = function() {
                        return (get_weights_matrix(list(.self)))
                       },
                       len = function() {
                        return (.self$obj$num())
                       },
                       id_conns = function() {
                        return (.self$obj$id_conns)
                       }))

setMethod("show", "SRMLayerClass", function(x) {
    cat("SRMLayer with ", x$obj$N, " neurons:\n", sep="")
    cat(" | ids:", c(x$ids()), "\n", sep=" ")
    cat(" | Pedge = ", x$prop$edge_prob, ", Num inhib. = ", x$prop$ninh, "\n", sep="")
})


