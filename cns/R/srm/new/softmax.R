#!/usr/bin/RScript



grab_epsp_sm <- function(t, x) {
    sapply(x, function(sp) sum(epsp(t-sp)))
}

rmax <- 1
SMNeuronLayer <- setRefClass("SMNeuronLayer", fields = list(weights = "list", id_conn = "vector", ids = "vector", len="vector", stochastic="logical"),
                                              methods = list(
                                              initialize = function(weights, id_conn, ids) {
                                                .self$weights <<- weights
                                                .self$id_conn <<- id_conn
                                                .self$ids <<- ids
                                                .self$len = length(weights)
                                                .self$stochastic = TRUE
                                              },
                                              rate = function(t, net) { Vectorize(function(t, net) {
                                                epsp = grab_epsp_sm(t, net[.self$id_conn])
                                                r_all = sapply(.self$weights, function(wk) wk %*% epsp)
                                                ee = exp(r_all)
                                                rmax*ee/sum(ee)
                                              }, "t")(t,net) },
                                              gradt = function(t, net, y) {
                                                target = rep(0, length(ids))
                                                target[y] = 1
                                                epsp = grab_epsp_sm(t, net[id_conn])
                                                all_r = rate(t, net)
                                                epsp %*% t(all_r * (target - all_r)) - 0.001*do.call(cbind, weights)
                                              },
                                              grad = function(T0, Tmax, net, target_set) {
                                                gr = adaptIntegrate( function(ti) .self$gradt(ti, net, target_set$class), T0, Tmax, fDim=length(unlist(.self$weights)), tol=1e-02, absError=1e-02, maxEval=100)$integral
                                                split(gr, ceiling(seq_along(gr)/length(.self$id_conn)))
                                              })
                                              )

P <- function(nl, net, T0, Tmax) {
  p_at_spikes = sapply( lapply(1:length(nl$ids), function(id) nl$r(net[[ nl$ids[id] ]], net)[id,] ), prod)
  p_at_spikes * exp(-adaptIntegrate( function(t) nl$r(t, net), T0, Tmax, fDim=length(nl$ids))$integral)
}

learn_layer <- function(patterns, layers, run_options) {
  lengths = sapply(patterns, function(p) p$len)
  stopifnot(all(lengths == lengths[1]))
  
  M = lengths[1]
  
  N = sum(sapply(layers, function(l) l$len))
  id_m = seq(1, M)
  id_n = seq(M+1, M+N)
  
  all_n = M
  run_options$mode = "run"
  for(ep in 1:100) {
    gr = list()
    nnet = list()
    net = list()
    for(id_patt in 1:length(patterns)) {
      net[id_m] = patterns[[id_patt]]$data
      net[id_n] = null_pattern.N
      run_options$class = patterns[[id_patt]]$class
      
      c(net, layers, stat, mean_grad) := run_srm(layers, net, run_options)
      nnet[[id_patt]] = net
      target_set$class = patterns[[id_patt]]$class
      unet = t(stat[[1]]$u)
      gr[[id_patt]] = layers[[2]]$grad(run_options$T0, run_options$Tmax, unet, target_set)
    }
    for(g in gr) {
      invisible(sapply(1:layers[[2]]$len, function(i) layers[[2]]$weights[[i]] = layers[[2]]$weights[[i]] + run_options$learning_rate * g[[i]] ))
    }
    r1 = rowSums(sapply(seq(run_options$T0, run_options$Tmax, by=0.5), function(t) layers[[2]]$rate(t, nnet[[1]])))
    r2 = rowSums(sapply(seq(run_options$T0, run_options$Tmax, by=0.5), function(t) layers[[2]]$rate(t, nnet[[2]])))
    cat("r1: ", r1, " r2: ", r2, "\n")
  }
}

test <- function() {
  net = list()
  net[[1]] <- c(-Inf, 1,2)
  net[[2]] <- c(-Inf, 2)
  net[[3]] <- c(-Inf, 5)
  net[[4]] <- c(-Inf)
  net[[5]] <- c(-Inf, 5)
  
  net2 = list()
  net2[[1]] <- c(-Inf, 1.5)
  net2[[2]] <- c(-Inf, 1, 5)
  net2[[3]] <- c(-Inf, 2)
  net2[[4]] <- c(-Inf, 2)
  net2[[5]] <- c(-Inf, 5)
  
  nl = SMNeuronLayer(w = list(c(3,3,3,3,3), c(3,3,3,3,3)),
                     id_conn = c(1,2,3,4,5),
                     ids = c(6,7))
  
  r = sapply(seq(0, 20, by=0.5), function(t) nl$r(t, net))
  
  
  
  T0 = 0
  Tmax = 30
  rate = 0.1
  wl = length(nl$id_conn)
  for(ep in 1:30) {
    gr1 = adaptIntegrate( function(ti) nl$grad(ti, net, 1), T0, Tmax, fDim=length(unlist(nl$w)), tol=1e-02, absError=1e-02, maxEval=100)$integral
    gr2 = adaptIntegrate( function(ti) nl$grad(ti, net2, 2), T0, Tmax, fDim=length(unlist(nl$w)), tol=1e-02, absError=1e-02, maxEval=100)$integral
    gr1_r = rate*(gr1)
    gr2_r = rate*(gr2)
    gl1 = split(gr1_r, ceiling(seq_along(gr1_r)/wl))
    gl2 = split(gr2_r, ceiling(seq_along(gr2_r)/wl))
    nl$w = mapply("+", nl$w, gl1, SIMPLIFY=FALSE)
    nl$w = mapply("+", nl$w, gl2, SIMPLIFY=FALSE)
    
    r1 = rowSums(sapply(seq(T0, Tmax, by=0.5), function(t) nl$r(t, net)))
    r2 = rowSums(sapply(seq(T0, Tmax, by=0.5), function(t) nl$r(t, net2)))
    cat("r1: ", r1, " r2: ", r2, "\n")
  }
}
#              exp(  wk %*% grap_epsp(t, x) )
# rk(t, x) = ---------------------------------
#            sum exp( wl %*% grap_epsp(t, x) )
#             l
# Pk = prod rk(tfi, x) * exp( - int rk(t, x) dt)
#       i
# log Pk(y = [tfi] | x ) = sum log( rk(tfi, x) ) -   int rk(t, x) dt  
#                           i                      [0,T] 


# dlog Pk                           drk            1               drk
# ------ (y = [tfi] | x ) =    sum  -- (tfi, x)  -----    -   int  ---(t, x) dt 
# dwk                      i=[1,n]  dwk         rk(tfi, x)  [0,T]  dwk


# drk
# --- (x, t) = epsp(t) * rk(x, t) * ( 1 - rk(x, t) )
# dwk

# drk
# --- (x, t) = - epsp(t) * rk(x, t) rm(x, t)
# dwm
#
#
#
#
#
