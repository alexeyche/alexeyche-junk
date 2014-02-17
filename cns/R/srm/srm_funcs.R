
if(refr_mode == 'low') {
    u_abs <- -120 # mV
    u_r <- -50#-50 # mV
    trf <- 2.25 # ms
    trs <- 2 # ms
    dr <- 1 # ms
} else 
if(refr_mode == 'middle') {
    u_abs <- -150 # mV
    u_r <- -50#-50 # mV
    trf <- 3.25 # ms
    trs <- 3 # ms
    dr <- 1 # ms
} else
if(refr_mode == 'high') {
    u_abs <- -250 # mV
    u_r <- -70#-50 # mV
    trf <- 5.25 # ms
    trs <- 5 # ms
    dr <- 3 # ms
} else {
    cat(sprintf("Can't find refration mode %s\n", refr_mode))
}

# epsp <- Vectorize(function(s) {
#   if((s<0)||(s == Inf)) { return(0) }
#   e0*(exp(-s/tm)-exp(-s/ts))
# })
epsp <- Vectorize(function(s) {
  if((s<0)||(s == Inf)) { return(0) }
  exp(-s/tm)
})

# absolute(-100 mV) refractoriness with exponential restore
nu <- Vectorize(function(s) {
 if(s<0) { return(0)}
 if(s<dr) { return(u_abs) }
 u_abs*exp(-(s+dr)/trf)+u_r*exp(-s/trs)
})

#t <- seq(0, 50, by=0.1)
#plot(t, nu(t-25), type="l")

# stochastic threshold g(u):

g <- Vectorize(function(u) {
  ans=(beta/alpha)*(log(1+exp(alpha*(tr-u))) -alpha*(tr-u))   
  return(ans)
})

u_srm <- function(t, x, y, w) {
  e_syn <- 0
  for(i in 1:length(x)) {
    e_syn <- e_syn + w[i]*sum(epsp(t-x[[i]]))
  }
  u_rest + e_syn + sum(nu(t-y))
}

probf = function(u) {
  pr + (u -u_rest)*gain_factor
}

a_suppr = Vectorize(function(s) {
  if(s<0) return(1)
  return(1- exp(-s/ta))
})

