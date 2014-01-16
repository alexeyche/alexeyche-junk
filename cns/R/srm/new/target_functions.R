


first_spike_tf <- function(nspikes) {  
  function(nspike_id) { 
    nspikes[[nspike_id]][1]
  } 
}

random_spike_tf <- function(nspikes) {  
  function(nspike_id) { 
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(resample(nspikes[[nspike_id]], 1))
  } 
}
random_2spikes_tf <- function(nspikes) {  
  function(nspike_id) { 
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(resample(nspikes[[nspike_id]], min(length(nspikes[[nspike_id]]), 2)))
  } 
}

random_3spikes_tf <- function(nspikes) {  
  function(nspike_id) { 
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(resample(nspikes[[nspike_id]], min(length(nspikes[[nspike_id]]), 3)))
  } 
}

random_4spikes_tf <- function(nspikes) {  
  function(nspike_id) { 
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(resample(nspikes[[nspike_id]], min(length(nspikes[[nspike_id]]), 4)))
  } 
}

random_5spikes_tf <- function(nspikes) {  
  function(nspike_id) { 
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(resample(nspikes[[nspike_id]], min(length(nspikes[[nspike_id]]), 5)))
  } 
}

full_spike_tf <- function(nspikes) { 
  function(nspike_id)  {
    if(length(nspikes[[nspike_id]]) == 0) return(NULL)
    return(nspikes[[nspike_id]])
  }
}