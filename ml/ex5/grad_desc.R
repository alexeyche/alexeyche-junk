#!/usr/bin/RScript

source("hypothesis.R")

grad <- function(x, y, theta) {
    m <- nrow(x)
    t((1/m) * t(x) %*% ( h(x,theta) - y ))
}

grad.descent <- function(x, y, maxit, alpha, lambda) {
    m <- nrow(x)
    n <- ncol(x)
    theta <- matrix(rep(1,n), nrow=1)
    reg <- t(rbind(0, as.matrix(rep(lambda/m,n-1))) )
    for(i in 1:maxit) {
#        print(err(x, y, theta))
        theta <- theta - alpha * ( grad(x, y, theta) + reg * theta )   
    }
    return(theta)
}


#source("load_data.R")
#source("featureCook.R")
#x <- cookFeature(x.data, 3)         
#n <- ncol(x)
#theta <- grad.descent(x, y, 5000, 0.02, 0.1)    
 


