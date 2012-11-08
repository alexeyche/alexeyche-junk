#!/usr/bin/RScript

n <- 2
m <- 1000
x <- matrix(cbind(2/3*rnorm(m, mean=0, sd=0.5),2/3*rnorm(m, mean=0, sd=2)), ncol = n, nrow = m)

E <- cov(x[,1:2]) 
mu <- matrix(apply(x, 2, mean), ncol = 1)

N <- function(x, mu, E) {
    (2*pi)^(-n/2) * det(E)^(1/2) * exp(-1/2 * t(x-mu) %*% solve(E) %*% (x-mu) )
}
x1.plot <- seq(-2,2, length.out=m)
x2.plot <- seq(-2,2, length.out=m)
p <- NULL
for(i in 1:m) {
    x_v <- matrix(c(x1.plot[i],x2.plot[i]), ncol=1)
    p <- rbind(p, N(x_v, mu, E))    
}




