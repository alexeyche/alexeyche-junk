
if(llh_depr_mode == 'low') {
    llh_depr_mode_value = 0.5
} else 
if(llh_depr_mode == 'middle') {
    llh_depr_mode_value = 0.15
    llh_factor = 1.25
} else
if(llh_depr_mode == 'high') {
    llh_depr_mode_value = 0.03
    llh_factor = 1.5
} else
if(llh_depr_mode == 'no') {
    llh_depr_mode_value = Inf
    llh_factor = 1
} else {
  cat(sprintf("Can't find depression mode %s\n", llh_depr_mode))
  q()
}

grad_func <- function(neurons, T0, Tmax, net, target_set) {
  id_n = neurons$ids #sapply(neurons, function(n) n$id)
  
  nspikes = lapply(net[id_n], function(sp) { 
    left = findInterval(T0, sp)+1
    right = findInterval(Tmax, sp, rightmost.closed=TRUE)
    if(left<=right) sp[left:right]
  })  

  spikes_survived <- NULL
  nspikes = lapply(1:length(id_n), function(ni) { 
   if(!is.null(nspikes[[ni]])) {
     if( (length(nspikes[[ni]])/(Tmax-T0)) > llh_depr_mode_value) {
       probs = g(neurons$u_one(ni, nspikes[[ni]], net))
       most_likely = probs>= (mean(probs)*llh_factor)
       new_nspikes = nspikes[[ni]][most_likely]        
       spikes_survived <<- c(spikes_survived, length(new_nspikes)/length(nspikes[[ni]]) )
       new_nspikes
     } else {
       spikes_survived <<- c(spikes_survived, 1)
       nspikes[[ni]]
     }
   }
  })
  
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
        #exc = rep(1, length(neurons$weights[[id_number]]))
        #exc[neurons$weights[[id_number]]<0] = - 1
        sapply(neurons$id_conns[[id_number]], function(idc) {
          if(!is.null(nspikes[[id_number]])) {            
            sum( left_part[[id_number]]*grab_epsp(nspikes[[id_number]], net[[idc]]))
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
  return(list(mapply("+", spike_part, int_part, SIMPLIFY=FALSE), spikes_survived))
}

grad_func_new = function(neurons, T0, Tmax, net) {
  int_options =list(T0=T0, Tmax=Tmax, dim=sum(sapply(neurons$id_conns, length)),quad=256)
  int_out = integrateSRM_epsp(neurons, int_options , net, constants)$out
  int_part = list()
  iter=1
  for(id in 1:neurons$len) {
    int_part[[id]] = int_out[iter:(iter+length(neurons$id_conns[[id]])-1)]
    iter = iter + length(neurons$id_conns[[id]])
  }  
  Y = sp_in_interval(net[neurons$ids], T0, Tmax)
  spike_part = lapply(1:neurons$len, function(ni) {
    spM = sapply(Y[ni], function(sp) probf(neurons$u_one(ni, sp, net)))
    epspM = neurons$epsp_fun_one(Y[[ni]], net, ni)
    div_of_vals = sapply(1:nrow(epspM), function(ri) epspM[ri,]/spM[ri])
    rowSums(div_of_vals)
  })  
  grad = mapply("-", spike_part, int_part, SIMPLIFY=FALSE)
  #grad = lapply(grad, function(gr) gr*gain_factor)
  return(grad)
  #py = neurons$P(T0, Tmax, net)
  #return(lapply(1:neurons$len, function(ni) grad[[ni]]*py[ni]))
}

