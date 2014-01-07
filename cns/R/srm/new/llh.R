
grad <- function(neurons, T0, Tmax, net, first_spike=TRUE) {
  
  
  id_n = sapply(neurons, function(n) n$id)
  
  nspikes = lapply(net[id_n], function(sp) { 
    left = findInterval(T0, sp)+1
    right = findInterval(Tmax, sp, rightmost.closed=TRUE)
    if(left<=right) sp[left:right]
  })
  if(first_spike)
    nspikes = lapply(nspikes, function(sp) sp[1])
  
  left_part = lapply(1:length(id_n), function(number) {  
              if(!is.null(nspikes[[number]])) {
                u = neurons[[number]]$u(nspikes[[number]], net) 
                #cat("n(", neurons[[number]]$id, ")u=",u,"\n")
                (p_stroke(u)/g(u))
              }
  })

  spike_part = sapply(1:length(id_n), function(id_number) {
        sapply(neurons[[id_number]]$id_conn, function(idc) {
          if(!is.null(nspikes[[id_number]])) {
            sum(left_part[[id_number]]*grab_epsp(nspikes[[id_number]], net[[idc]]))
          } else {
            0
          }
      })
  })
  not_fired = sapply(nspikes, is.null)
  int_options = list(T0 = T0, Tmax=Tmax)
  sfExport('int_options')
  sfExport('net')
  sfExport('not_fired')
  sfExport('neurons')
  
  int_part = sfSapply(1:length(neurons), function(ni)  { 
    if(not_fired[ni] == FALSE) {
      integrateSRM(constants, int_options, neurons[[ni]]$id, neurons[[ni]]$id_conn, neurons[[ni]]$w, net)
    } else {
      rep(0, length(neurons[[ni]]$w))
    }
  })

  return(spike_part + int_part)
}
