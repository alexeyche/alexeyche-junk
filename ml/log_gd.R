#!/usr/bin/RScript


data <- read.csv("ex2/ex2data1.txt", sep=",", header=F)

x <- cbind(data$V1,data$V2)
y <- cbind(data$V3)
m <- nrow(x)
n <- ncol(x)

# normalize
means <- apply(x,2,mean)
sds <- apply(x,2,sd)
x.n <- t((t(x) - means )/ sds)

# 1 feature

x.n <- cbind(rep(1,m),x.n)
x <- cbind(rep(1,m),x)

g <- function(z) {
    1/(1+exp(-z))
}

h_sigm <- function (x_val,theta) {
    g(x_val %*% t(theta))
}

grad <- function (x, y, theta) {
    gradient <- (1/m) * (t(x) %*% ( h_sigm(x,theta) - y ))
    return(t(gradient))
}

grad.descent <- function (x, y, maxit, alpha) {
    theta <- matrix(rep(1,n+1), nrow=1)
    for( i in 1:maxit ) {
        g <- grad(x, y, theta)
        theta <- theta - alpha * g       
    }
    return(theta)
}

theta <- grad.descent(x.n, y, 1000, 0.01)

# th1 + th2*x1 + th3*x2 = 0
plot_func <- function (x_val) {
       
}

plot(subset(x.n[,2:3], y == 0), col="red", type="p")
points(subset(x.n[,2:3], y == 1), col="green")
x_val = seq(-2,2,by=0.1)
lines(x_val,(-theta[2]*x_val -theta[1])/theta[3], col="black")

