#!/usr/bin/RScript

data <- read.csv("ex1/ex1data1.txt",sep=",",header=F)

x1 <- data$V1
y <- as.matrix(data$V2)
#x2 <- x1^2

m <- length(y)

x0 <- rep(1, m)

x <- as.matrix(cbind(x0,x1))

# implementation

h <- function (x_val,theta) {
    x_val %*% t(theta)
}

grad <- function (x, y, theta) {
    gradient <- (1/m) * (t(x) %*% ( h(x,theta) - y ))
    return(t(gradient))
}

cost_function <- function(x,y,theta) {
    1/(2*length(x)) * sum((h(x,theta) - y)^2)
}

thetas <- matrix()
grad.descent <- function (maxit, alpha) {
    theta <- matrix(rep(1,2), nrow=1)
    thetas <<- theta
    for( i in 1:maxit ) {
        g <- grad(x, y, theta)
        theta <- theta - alpha * g       
        thetas <<- as.matrix(rbind(thetas,theta))
    }
    return(theta)
}

theta <- grad.descent(1000,0.01)

# visualization 

plot_f1 <- function(x, theta) {
    theta[1] + theta[2]*x + theta[3]*x^2
}

plot_f2 <- function(x, theta) {
    theta[1] + theta[2]*x
}

#plot(x[,2],y)
#lines(sort(x[,2]),plot_f1(sort(x[,2]),theta), col="red")
#lines(sort(x[,2]),sort(plot_f2(x[,2],theta)), col="green")

