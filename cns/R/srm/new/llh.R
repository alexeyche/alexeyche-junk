
grad <- function(neurons, T0, Tmax, net, class, target_set) {
  id_n = sapply(neurons, function(n) n$id)
  
  nspikes = lapply(net[id_n], function(sp) { 
    left = findInterval(T0, sp)+1
    right = findInterval(Tmax, sp, rightmost.closed=TRUE)
    if(left<=right) sp[left:right]
  })

  nspikes = lapply(1:length(nspikes), target_set$target_function_gen(nspikes, class, T0, Tmax))
  
  left_part = lapply(1:length(id_n), function(number) {  
              if(!is.null(nspikes[[number]])) {
                u = neurons[[number]]$u(nspikes[[number]], net) 
                #cat("n(", neurons[[number]]$id, ")u=",u,"\n")
                (p_stroke(u)/g(u))
              }
  })

  spike_part = lapply(1:length(id_n), function(id_number) {
        sapply(neurons[[id_number]]$id_conn, function(idc) {
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
    not_fired = rep(FALSE, length(neurons))
  }    
  int_options = list(T0 = T0, Tmax=Tmax)
  sfExport('int_options')
  sfExport('net')
  sfExport('not_fired')
  sfExport('neurons')
  
  int_part = sfLapply(1:length(neurons), function(ni)  { 
    if(not_fired[ni] == FALSE) {
      integrateSRM(constants, int_options, neurons[[ni]]$id, neurons[[ni]]$id_conn, neurons[[ni]]$w, net)
    } else {
      rep(0, length(neurons[[ni]]$w))
    }
  })

  return(mapply("+", spike_part, int_part, SIMPLIFY=FALSE))
}
