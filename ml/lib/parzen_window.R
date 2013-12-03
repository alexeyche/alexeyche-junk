#!/usr/bin/env RScript

K <- function(r) {
  ignore <- abs(r) > 1
  r <- 3/4 * (1-r^2)
  r[ignore] <- 0
  return(r)
}

# x     - x1  x2  .. xN   - vector for point
# x.all - x11 x12 .. x1N  - feature matrix
#         x21 x22 .. x2N
#         ...
#         xM1 xM2 .. xMN  
# h - window
parzen_window <- function(x, x.all, h) {
  kernels_pr <- 1
  n <- ncol(x.all)
  m <- nrow(x.all)  
  # it is product on j
  for(j in 1:n) {
    kernels_pr <- kernels_pr * K(abs(x.all[,j] - x[,j])/h)/h
  }
  # it is sum on i
  sum(kernels_pr)/m
}

eval_parzen_window <- function(x.all, h) {
    p <- NULL
    for(i in 1:nrow(x.all)) {
        x.i <- matrix(x.all[i,], nrow=1)
        p <- rbind(p, parzen_window(x.i, x.all, h))
    }
    return(p)
}

test_parzen_window <- function() {
    library(scatterplot3d)
    x.t <- as.matrix( cbind(rnorm(2000),rnorm(2000)) )
    par(mfrow=c(3,3))
    for(i in 1:9) {
        p <- eval_parzen_window(x.t, i*0.1)
        scatterplot3d(x.t[,1],x.t[,2], p)
    }
}
