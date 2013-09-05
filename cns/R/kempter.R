#!/usr/bin/RScript

# epsp parameters:

source('sys.R')

t0 <- 10 # ms

# W parameters:
nu <- 10^-5
t_syn <- 5 # ms
t_pos <- 1 # ms
t_neg <- 10 # ms
Apos <- 1
Aneg <- -1
t_pos_ <- t_syn*t_pos/(t_syn+t_pos)
t_neg_ <- t_syn*t_neg/(t_syn+t_neg)

T <- 1000

epsp <- Vectorize(function(s) {
  if(s<0) { return(0)}
  (s/(t0^2)) * exp(-s/t0)
})
#s<-seq(0,100, length.out=1000); s<-plot(s,epsp(s), type="l")
#  integrate(epsp, 0,1000)


W <- function(s) {
  neg_ind <- s<=0
  pos_ind <- s>0
  if(length(neg_ind>0)) {
    s[neg_ind] <- exp(s[neg_ind]/t_syn)*(Apos*(1-s[neg_ind]/t_pos_) + Aneg*(1-s[neg_ind]/t_neg_))
  } 
  if(length(pos_ind>0)) {
    s[pos_ind] <- Apos*exp(-s[pos_ind]/t_pos) + Aneg*exp(-s[pos_ind]/t_neg)  
  }
  return(nu*s)
}
#s <- seq(-40,60,length.out=1000); plot(s, W(s), type="l")

aver <- function(f) {
  function(t) {
    integrate(f, t,t+T)$value
  }
}

# J parameters:

v0 <- 0
N <- 100
M1 <- N/2
M2 <- N/2
w_in <- nu
w_out <- -1.0475*nu
J <- abs(rnorm(N, sd=0.01))

lambda_in <- function(i, t) 10

lambda_in.big <- function(i,t) { 
  integrate(function(s) { epsp(s)*lambda_in(i,t-s) },0, Inf)$value
}

S_out_av <- function(t) { 
  v0 + sum_over(function(i) J[i]*lambda_in.big(i,t), N)
}

corr_av_i <- function(i,t,s) {
  lambda_in(i,t+s)*(v0 + J[i]*epsp(-s)+sum_over(function(j) J[j]*lambda_in.big(i,t), N) )
}




