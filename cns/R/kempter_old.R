#!/usr/bin/RScript
source('sys.R')
source('poisson_process.R')

# epsp parameters:



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
  f <- Vectorize(f)
  function(t) {
    integrate(f, t,t+T)$value/T
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

lambda_per <- function(t) 10/1000*cos(t/1000*2*40*2*pi) + 10/1000

lambda_in <- Vectorize(function(i, t) { if(i<M1) 10/1000 else lambda_per(t) })

lambda_in.big <- function(i,t) { 
  integrate(function(s) { epsp(s)*lambda_in(i,t-s) },0, Inf)$value
}

S_out_av <- function(t) { 
  v0 + sum_over(function(i) J[i]*lambda_in.big(i,t), N)
}

s_in_s_out_av <- function(i,s,t) {
  lambda_in(i,t+s)*(v0 + J[i]*epsp(-s)+sum_over(function(j) J[j]*lambda_in.big(i,t), N) )
}

corr <- function(i, s, t) {
  sum_over(function(j) J[j]*
            aver(function(t) lambda_in(i,t+s)*lambda_in.big(j,t))(t)
           ,N) + aver(function(t) lambda_in(i,t+s))(t)*(v0+J[i]*epsp(-s))
}

sss <- NULL; for(ti in tic) { sss <- c(sss, S_out_av(ti)); cat(ti,"\n") }

Q <- function(i, j, t) {
  lam  <-function(s, t) { 
    c_out <- NULL
    for(s_i in s) {
      c_out <- c(c_out, aver((function(t.0) { lambda_in(i, t.0+s_i)*lambda_in.big(j,t.0) }))(t))
    }
    c_out
  }
  integrate(function(s) { Vectorize(W(s)*lam(s,t)) },-Inf,Inf)$value
}


