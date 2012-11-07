#!/usr/bin/RScript


h <- function(x, theta) {
    x %*% t(theta)
}

err <- function(x, y, theta) {
    m <- nrow(x)
    y.model <- h(x, theta)
    error <- sum((y - y.model)^2)*1/m
    return(error)
}

grad <- function(x,y,theta) {
    m <- nrow(x)
    gradient <- 1/m * t(x) %*% ( h(x,theta) - y )
    t(gradient)
}

grad.descent <- function(x, y, maxit, alpha, lambda) {
    n <- ncol(x)
    m <- nrow(x)
    theta <- matrix(rep(1,n), nrow = 1 )
    reg <- t(rbind(0, as.matrix(rep(lambda/m,n-1))) )
    for(i in 1: maxit) {
#        print(err(x, y, theta))
        theta <- theta - alpha * ( grad(x, y, theta) + reg * theta )   
    }
    return(theta)
}

test <- function() {
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
}

test_real <- function() {
    source('load_data.R')
    degree <- 5
    x.m <- mapFeature(x.n, degree)
    y.m <- y.n
    
    theta <- grad.descent(x.m, y.m, 5000, 0.05, 0.8)
    source('plot_data.R')
    plot_hypothesis(theta, degree, x.n, y.n)
    points(xtest.n, ytest.n, col="blue")
}



