#!/usr/bin/RScript


h <- function(x, theta) {
    x %*% t(theta)
}

load_test_data <- function() {
    xt <<- matrix(c(1,2,3,4,5), nrow=5, ncol=5)
    yt <<- matrix(c(2,4,6,8,10), ncol=1)
    theta <<- t(as.matrix(c(1,1,1,1,1)))
}
test_h <- function() {
    load_test_data()
    print(h(xt,theta))
}

grad <- function(x,y,theta) {
    m <- nrow(x)
    gradient <- 1/m * t(x) %*% ( h(x,theta) - y )
    t(gradient)
}

test_grad <- function() {
    load_test_data()    
    print(grad(xt,yt,theta))
}

err <- function(x, y, theta) {
    m <- nrow(x)
    y.model <- h(x, theta)
    error <- sum((y - y.model)^2)*1/m
    return(error)
}

grad.descent <- function(x, y, maxit, alpha, lambda) {
    n <- ncol(x)
    m <- nrow(x)
    theta <- matrix(rep(1,n), nrow = 1 )
    reg <- t(rbind(0, as.matrix(rep(lambda/m,n-1))) )
    for(i in 1: maxit) {
        print(err(x, y, theta))
        theta <- theta - alpha * ( grad(x, y, theta) + reg * theta )   
    }
    return(theta)
}

set.seed(1)
"gen" <- function(x){  
    return(30*x^2 + 5*x + 10 + rnorm(length(x))*0.1)  
}

source('featureCook.R')

X <- as.matrix(runif(10))
Y <- gen(X)

X <- featureNormalize(X)
Y <- featureNormalize(Y)

X.m <- mapFeature(X, 2)

theta <- grad.descent(X.m, Y, 2000, 0.01, 0.1)

X.plot <- as.matrix(seq(-1.5,1.5, length.out = 300))
Y.plot <- h(mapFeature(X.plot,2), theta)

plot(X,Y)
lines(X.plot,Y.plot)





