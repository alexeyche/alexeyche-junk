
grad_func <- function(neurons, T0, Tmax, net, target_set) {
  id_n = neurons$ids #sapply(neurons, function(n) n$id)
  
  nspikes = lapply(net[id_n], function(sp) { 
    left = findInterval(T0, sp)+1
    right = findInterval(Tmax, sp, rightmost.closed=TRUE)
    if(left<=right) sp[left:right]
  })  
  
  nspikes = lapply(1:length(id_n), function(ni) { 
    if(!is.null(nspikes[[ni]])) {
      if( (length(nspikes[[ni]])/(Tmax-T0)) > llh_depr) { #0.04666667) { #0.06)  {  #  >= 4 spikes
        probs = g(neurons$u_one(ni, nspikes[[ni]], net))
        most_likely = probs>=mean(probs)
        nspikes[[ni]][most_likely]
        #NULL
      } else {
        nspikes[[ni]]
      }
    }
  })
#   if(sum(sapply(nspikes, length)/(Tmax-T0)) > 0.2) {
#    nspikes = lapply(1:length(id_n), function(ni) { 
#        if(!is.null(nspikes[[ni]])) {
#          probs = g(neurons$u_one(ni, nspikes[[ni]], net))
#          most_likely = probs>=mean(probs)
#          nspikes[[ni]][most_likely]
#        }
#   })
#   }
  
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
  int_options = list(T0 = T0, Tmax=Tmax, dim=sum(sapply(neurons$id_conns, length)), quad=256)  
 
  grad = integrateSRM_vec(constants, int_options, neurons$ids, neurons$id_conns, neurons$weights, net)$out
  
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
  return(mapply("+", spike_part, int_part, SIMPLIFY=FALSE))
}
