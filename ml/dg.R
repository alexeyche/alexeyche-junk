#!/usr/bin/RScript

data <- read.csv("ex1/ex1data1.txt",sep=",",header=F)

x1 <- data$V1
y <- as.matrix(data$V2)
x2 <- sqrt(x1)

m <- length(y)

x0 <- rep(1, m)

x <- as.matrix(cbind(x0,x1,x2))

# implementation

h <- function (x_val,theta) {
    x_val %*% t(theta)
}

grad <- function (x, y, theta) {
    gradient <- (1/m) * (t(x) %*% ( h(x,theta) - y ))
    return(t(gradient))
}

grad.descent <- function (maxit, alpha) {
    theta <- matrix(rep(0,3), nrow=1)
    for( i in 1:maxit ) {
        g <- grad(x, y, theta)
        theta <- theta - alpha * g        
    }
    return(theta)
}

theta <- grad.descent(100,0.001)
plot(x[,2],y)
lines(x[,2],h(x,theta))
