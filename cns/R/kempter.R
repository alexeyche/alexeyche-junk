#!/usr/bin/RScript
source('sys.R')
source('poisson_process.R')
source('kempter_funcs.R')


# J parameters:
nu <- 10^-5
v0 <- 0
N <- 50
M1 <- N/2
M2 <- N/2
w_in <- nu
w_out <- -1.0475*nu

#Jstart <- c(rep(0.1, N/4), rep(0.08, N/4), rep(0.06, N/4), rep(0.04, N/4))
Jstart <-  abs(rnorm(N, mean=0.07, sd=0.05))
Jstart[Jstart>0.1] <- 0.1
#T <- 100
#dt <- 0.1
#t <- seq(0,T, by = dt)

lambda_per <- function(t) 10/1000*cos(t/1000*2*40*2*pi) + 10/1000

lambda_in <- 10 # Vectorize(function(i, t) { if(i<M1) 10/1000 else lambda_per(t) })

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
Q1 <-  matrix(delta_wn*c_part, nrow=N/2, ncol=N)
Q2 <-  matrix(0, nrow=N/2, ncol=N)
Q <- rbind(Q1, Q2)

c_m <- matrix(0, nrow=N, ncol=N)
diag(c_m) <- c_koeff

e <- matrix(rep(1,N),ncol=N, nrow=1)
b <- matrix(rep(b_koeff,N),ncol=1, nrow=N)

a <- matrix(rep(a_koeff,N), ncol=1, nrow=N)

require(deSolve)
emerge_weights <- function (time, J, pars) {
    with(as.list(c(pars)), {
        dJ <- a + (b %*% e + c_m + Q) %*% J
        dJ[J>Jbound] <- 0
        dJ[J<=0] <- 0
        return(list(c(dJ)))
    })        
}

pars = c(a = a, b = b, e = e, c_m = c_m, Q = Q, Jbound = 0.1)

tStart <- 0
tEnd <- 50000
dt <- 10

out <- ode(func=emerge_weights, y=Jstart, parms=pars, times = seq(tStart, tEnd, by = dt), method="euler")
Jend <- out[nrow(out), 2:(N+1)]


plot(out[,1], out[,2], type="l", ylim=c(0, max(out[,2:N])))
for(i in 3:(N+1)) {
  lines(out[,1], out[,i])
}





