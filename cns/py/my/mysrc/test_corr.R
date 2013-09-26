#!/usr/bin/RScript

library(mvtnorm)
rho <- 0.3
mu <- c(10,10)
phi <- c(0.2,0.8)
theta <- c(0.3,-0.7)
d <- ts(matrix(0,ncol=2,nrow=50))
e <- ts(rmvnorm(50,sigma=cbind(c(0.5,rho),c(rho,0.5))))
for(i in 2:50)
  d[i,] <- mu + phi*d[i-1,] - theta*(e[i-1,]+e[i,])

plot(d)
