
require(cubature)

integrand <- function(t, net, neurons) {
  syn_epsp = sapply(net, function(sp) sum(epsp(t-sp)))  
  ps = p_stroke(neurons$u(t, net))
  unlist(lapply(1:neurons$len, function(id) syn_epsp[neurons$id_conns[[id]] ] * ps[id] )  )
}

grad_func <- function(neurons, T0, Tmax, net, target_set) {
  id_n = neurons$ids #sapply(neurons, function(n) n$id)
  
  nspikes = lapply(net[id_n], function(sp) { 
    left = findInterval(T0, sp)+1
    right = findInterval(Tmax, sp, rightmost.closed=TRUE)
    if(left<=right) sp[left:right]
  })  

  nspikes = lapply(1:length(nspikes), target_set$target_function_gen(nspikes))
  
  left_part = lapply(1:length(id_n), function(number) {  
              if(!is.null(nspikes[[number]])) {
                u = neurons$u_one(number, nspikes[[number]], net) 
                #cat("n(", neurons[[number]]$id, ")u=",u,"\n")
                
                ans = (p_stroke(u)/g(u))
                ans[ans>5] = 5
                return(ans)
              }
  })
  
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
 
 
  
 grad = adaptIntegrate(function(x) integrand(x, net, neurons), T0, Tmax, tol=1e-03, fDim=sum(sapply(neurons$id_conns, length)), maxEval=100)$integral
 int_part = list()
 iter=1
 for(id in 1:neurons$len) {
   if(!not_fired[id]) {
     int_part[[id]] = -grad[iter:(iter+length(neurons$id_conns[[id]])-1)]
   } else {
     int_part[[id]] = rep(0, length(neurons$id_conns[[id]]))
   }
   iter = iter + length(neurons$id_conns[[id]])
 } 
#   sfExport('int_options')
#   sfExport('net')
#   sfExport('not_fired')
#   neuronsl = neurons$to_list()
#   sfExport('neuronsl')
#   int_part = sfLapply(1:length(neuronsl), function(ni)  { 
#   if(not_fired[ni] == FALSE) {
#    integrateSRM(constants, int_options, neuronsl[[ni]]$id, neuronsl[[ni]]$id_conn, neuronsl[[ni]]$w, net)
#   } else {
#    rep(0, length(neuronsl[[ni]]$w))
#   }
#   })
  #int_part[,not_fired] = 0
  return(mapply("+", spike_part, int_part, SIMPLIFY=FALSE))
}
