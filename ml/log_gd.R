#!/usr/bin/RScript


data <- read.csv("ex2/ex2data1.txt", sep=",", header=F)

x <- cbind(data$V1,data$V2,data$V1^2)
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

cost_func <- function (x_val,y_val,theta) {
    sum(-y_val*log(h_sigm(x_val,theta)) - (1-y_val)*log(1-h_sigm(x_val,theta)))/m
}

grad <- function (x, y, theta) {
    gradient <- (1/m) * (t(x) %*% ( h_sigm(x,theta) - y ))
    return(t(gradient))
}

grad.descent <- function (x, y, maxit, alpha) {
    theta <- matrix(rep(1,n+1), nrow=1)
    for( i in 1:maxit ) {
        str(cost_func(x.n,y,theta))
        g <- grad(x, y, theta)
        theta <- theta - alpha * g       
    }
    return(theta)
}

theta <- grad.descent(x.n, y, 10000, 0.5)

# th1 + th2*x1 + th3*x2 + th4*x1^2 = 0
# x2 = -th1 - th2*x1 - th4*x1^2 / th3 

plot_func <- function (x_val) {
#      theta[1] + theta[2]*x_val[1] + theta[3]*x_val[2] + theta[4]*x_val[1] 
}



plot(subset(x.n[,2:3], y == 0), col="red", type="p")
points(subset(x.n[,2:3], y == 1), col="green")
x_val = seq(-2,2,by=0.1)
lines(x_val,(-theta[2]*x_val -theta[1] - theta[4]*x_val^2)/theta[3], col="black")

