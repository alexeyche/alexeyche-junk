
e0 <- 1.3 # mV
ts <- 3 # ms
tm <- 10 # ms
epsp <- Vectorize(function(s) {
  if((s<0)||(s == Inf)) { return(0) }
  e0*(exp(-s/tm)-exp(-s/ts))
})

# absolute(-100 mV) refractoriness with exponential restore
u_abs <- -150 # mV
u_r <- -50#-50 # mV
trf <- 3.25 # ms
trs <- 3 # ms
dr <- 1 # ms
nu <- Vectorize(function(s) {
 if(s<0) { return(0)}
 if(s<dr) { return(u_abs) }
 u_abs*exp(-(s+dr)/trf)+u_r*exp(-s/trs)
})

#t <- seq(0, 50, by=0.1)
#plot(t, nu(t-25), type="l")

# stochastic threshold g(u):
alpha <- 1.2
beta <- 1
tr <- -50 # mV
g <- Vectorize(function(u) {
  (beta/alpha)*(log(1+exp(alpha*(tr-u))) -alpha*(tr-u)) 
})

u_rest <- -70 # mV
u_srm <- function(t, x, y, w) {
  e_syn <- 0
  for(i in 1:length(x)) {
    e_syn <- e_syn + w[i]*sum(epsp(t-x[[i]]))
  }
  u_rest + e_syn + sum(nu(t-y))
}



