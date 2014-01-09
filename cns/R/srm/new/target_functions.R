

class_tf_complex <- function(nspikes, class, T0, Tmax) {  
  function(nspike_id) { 
    if(class == nspike_id) {
      if(length(nspikes[[nspike_id]]) == 0) 
        return(sort(T0+runif(5)*Tmax))
      return(nspikes[[nspike_id]])
    } else {
      return(NULL)
    }
  } 
}

class_tf <- function(nspikes, class, T0, Tmax) {  
  function(nspike_id) { 
    if(class == nspike_id) {
      return(nspikes[[nspike_id]])
    } else {
      return(NULL)
    }
  } 
}

first_spike_tf <- function(nspikes, class, T0, Tmax) {  
  function(nspike_id) { 
    nspikes[[nspike_id]][1]
  } 
}

random_spike_tf <- function(nspikes, class, T0, Tmax) {  
  function(nspike_id) { 
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(resample(nspikes[[nspike_id]], 1))
  } 
}
random_2spikes_tf <- function(nspikes, class, T0, Tmax) {  
  function(nspike_id) { 
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(resample(nspikes[[nspike_id]], min(length(nspikes[[nspike_id]]), 2)))
  } 
}

random_3spikes_tf <- function(nspikes, class, T0, Tmax) {  
  function(nspike_id) { 
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(resample(nspikes[[nspike_id]], min(length(nspikes[[nspike_id]]), 3)))
  } 
}

full_spike_tf <- function(nspikes, class, T0, Tmax) { 
  function(nspike_id) nspikes[[nspike_id]]
}