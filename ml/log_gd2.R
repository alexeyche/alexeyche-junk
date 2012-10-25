#!/usr/bin/RScript

data <- read.csv("ex2/ex2data2.txt", sep=",", header=F)

mapFeature <- function(x) {
    return(cbind(x,x[,1]^2,x[,2]^2))
}
x.data <- cbind(data$V1,data$V2)
y <- cbind(data$V3)

x <- mapFeature(x.data)


plot.log_data <- function(x,y) {
    plot(subset(x, y == 0), col="red")
    points(subset(x, y == 1), col="blue")
}

m <- nrow(x)

x <- cbind(rep(1,m),x)

n <- ncol(x)

cost_func <- function (x_val,y_val,theta) {
    sum(-y_val*log(h_sigm(x_val,theta)) - (1-y_val)*log(1-h_sigm(x_val,theta)))/m
}

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

grad.descent <- function (x, y, maxit, alpha, lambda) {
    theta <- matrix(rep(1,n), nrow=1)
    for( i in 1:maxit ) {
        str(cost_func(x,y,theta))
        g <- grad(x, y, theta)
        theta <- theta - alpha * ( g + lambda/m )      
    }
    return(theta)
}

theta <- grad.descent(x, y, 500, 0.1, 0)

x.plot <- cbind(seq(-2,2,by=0.1), seq(-2,2,by=0.1))
y.plot <- g(cbind(1,mapFeature(x.plot)) %*% t(theta))

y.plot <-  apply(y.plot, 1, function(y_val) { ifelse(y_val<0.5,0,1) })

plot.log_data(x.data,y)
par(new=TRUE)
plot.log_data(x.plot,y.plot)
