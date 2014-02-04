
source('constants.R')

stdp = Vectorize(function(s) {
  if(s<=0) {
    Aplus*exp(s/tplus)
  } else {
    -Aminus*exp(-s/tminus)   
  }
}) 
