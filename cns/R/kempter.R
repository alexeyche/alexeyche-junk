#!/usr/bin/RScript
source('sys.R')
source('poisson_process.R')
source('kempter_funcs.R')


# J parameters:
nu <- 10^-5
v0 <- 0
N <- 100
M1 <- N/2
M2 <- N/2
w_in <- nu
w_out <- -1.0475*nu
J <- abs(rnorm(N, sd=0.01))
T <- 100
dt <- 0.1
t <- seq(0,T, by = dt)

lambda_per <- function(t) 10/1000*cos(t/1000*2*40*2*pi) + 10/1000

lambda_in <- 10/1000 # Vectorize(function(i, t) { if(i<M1) 10/1000 else lambda_per(t) })

lambda_in.big <- function(i,t) { 
  integrate(function(s) { epsp(s)*lambda_in(i,t-s) },0, Inf)$value
}

W0 <- integrate(W, -Inf,Inf)$value
nu_in <- 10

a_koeff <- w_in*nu_in + v0*(w_out + nu_in*W0)
b_koeff <- nu_in*w_out

c_part_int <- function(s) { W(s)*epsp(-s) }
c_part <- integrate(Vectorize(c_part_int),-Inf,Inf)$value
c_koeff <- nu_in*c_part

delta_wn <- 0.1
Q <-  matrix(delta_wn*c_part, nrow=N, ncol=N)
c_m <- matrix(0, nrow=N, ncol=N)
diag(c_m) <- c_koeff

e <- matrix(rep(1,N),ncol=N, nrow=1)
b <- matrix(rep(b_koeff,N),ncol=1, nrow=N)

