#!/usr/bin/RScript


data <- read.csv("ex2/ex2data2.txt", sep=",", header=F)

mapFeature <- function(x) {
    x1 <- x[,1]
    x2 <- x[,2]
    return(cbind(x, x1*x2, x1^2, x2^2,x1^2*x2^2, x1^3, x2^3, x1^3*x2^3,x1^4, x2^4,x1^4*x2^4))
}

x.data <- cbind(data$V1,data$V2)
y <- cbind(data$V3)

x <- mapFeature(x.data)


plot.log_data <- function(x,y) {
    plot(subset(x, y < 0.5), col="red")
    points(subset(x, y >= 0.5), col="blue")
}

m <- nrow(x)

x <- cbind(rep(1,m),x)

n <- ncol(x)

theta <- matrix(rep(-1,n), nrow=1)

sigm <- function(z) {
    1/(1+exp(-z))
}

h <- function(x,theta) {
    sigm( x %*% t(theta) )   # ncol(x) = ncol(theta)
}

err <- function(x, y, theta) {
    y.model <- h(x, theta)
    err <- sum((y - y.model)^2) / length(y)
    return(err)
}

grad <- function(x, y, theta) {
    t((1/m) * t(x) %*% ( h(x,theta) - y ))
}

grad.descent <- function(x, y, theta, maxit, alpha, lambda) {
    reg <- t(rbind(0, as.matrix(rep(lambda/m,n-1))) )
    for(i in 1:maxit) {
        str(err(x, y, theta))
        theta <- theta - alpha * ( grad(x, y, theta) + reg * theta )   
    }
    return(theta)
}



theta <- grad.descent(x, y, theta, 10000, 0.3, 1)

x.plot <- cbind(seq(-2,2,by=0.01), seq(-2,2,by=0.01))
x.plot <- as.matrix(expand.grid(x.plot[,1],x.plot[,2]))
y.plot <- h( cbind(1,mapFeature(x.plot)), theta )
x.plot.l <- subset(x.plot,y.plot < 0.506 & y.plot > 0.494)
plot(subset(x.data, y >= 0.5), col="blue", ylim=c(-1.5,1.5), xlim=c(-1.5,1.5))
points(subset(x.data, y < 0.5), col="red")
points(x.plot.l, type="p", pch=16)
