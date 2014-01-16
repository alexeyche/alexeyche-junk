
integrand <- function(t, net, neurons) {
  syn_epsp = sapply(net[neurons$id_conns[[1]]], function(sp) sum(epsp(t-sp)))  
  c(sapply(p_stroke(neurons$u(t, net)), function(x) x*syn_epsp))
}

grad_func <- function(neurons, T0, Tmax, net, target_set) {
  id_n = neurons$ids #sapply(neurons, function(n) n$id)
  
  nspikes = lapply(net[id_n], function(sp) { 
    left = findInterval(T0, sp)+1
    right = findInterval(Tmax, sp, rightmost.closed=TRUE)
    if(left<=right) sp[left:right]
  })

  #nspikes = lapply(1:length(nspikes), target_set$target_function_gen(nspikes))
  
  left_part = lapply(1:length(id_n), function(number) {  
              if(!is.null(nspikes[[number]])) {
                u = neurons$u_one(number, nspikes[[number]], net) 
                #cat("n(", neurons[[number]]$id, ")u=",u,"\n")
                
                ans = (p_stroke(u)/g(u))
                #ans[ans == Inf] = 0
                return(ans)
              }
  })
  if(any(sapply(left_part, function(x) Inf %in% x))) {
    stop()
  }
  
  spike_part = lapply(1:length(id_n), function(id_number) {
        sapply(neurons$id_conns[[id_number]], function(idc) {
          if(!is.null(nspikes[[id_number]])) {
            sum(left_part[[id_number]]*grab_epsp(nspikes[[id_number]], net[[idc]]))
          } else {
            0
          }
      })
  })
  if(!target_set$depress_null) {
    not_fired = sapply(nspikes, is.null)
  } else {
    not_fired = rep(FALSE, neurons$len)
  }    
  int_options = list(T0 = T0, Tmax=Tmax)
  
  
#  grad = adaptIntegrate(function(t) integrand(t, net, neurons), T0, Tmax, tol=1e-03, fDim=length(id_n)*length(neurons$id_conns[[1]]), maxEval=1000)
#  int_part = matrix(-grad$integral, nrow = length(neurons$id_conns[[1]]), ncol=length(id_n))
#  int_part = lapply(seq_len(ncol(int_part)), function(ci) int_part[,ci])
  sfExport('int_options')
  sfExport('net')
  sfExport('not_fired')
  neuronsl = neurons$to_list()
  sfExport('neuronsl')
  int_part = sfLapply(1:length(neuronsl), function(ni)  { 
  if(not_fired[ni] == FALSE) {
   integrateSRM(constants, int_options, neuronsl[[ni]]$id, neuronsl[[ni]]$id_conn, neuronsl[[ni]]$w, net)
  } else {
   rep(0, length(neurons[[ni]]$w))
  }
  })
  #int_part[,not_fired] = 0
  return(mapply("+", spike_part, int_part, SIMPLIFY=FALSE))
}
