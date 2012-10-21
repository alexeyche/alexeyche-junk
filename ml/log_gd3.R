#!/usr/bin/RScript


data <- read.csv("ex2/ex2data2.txt", sep=",", header=F)

x <- cbind(rep(1,length(data$V1)),data$V1,data$V2,data$V1*data$V2,data$V1^2,data$V2^2)
y <- cbind(data$V3)

n <- length(x[1,])
m <- length(x[,1])

g <- function(x) {
    1/(1 + exp(-x))
}

h_sigm <- function(x,theta) {
    g(x %*% t(theta))
}


grad <- function (x,y,theta) {
    t((1/m) * t(x) %*% (h_sigm(x,theta)-y))
}

grad_descent <- function(x,y,it,alpha) {
    theta <- matrix(-1, nrow=1, ncol=n)
    reg = t(rbind(0, as.matrix(rep(1/m,n-1))))
    for(i in 1:it) {
        theta <- theta - alpha * ( grad(x,y,theta) - reg)
    }
    return(theta)
}

theta <- grad_descent(x,y,5000,1)

#theta <- matrix(c(1,1,1,1,1), nrow=1)
# th1 + th2*x^2 + th3*y^2 = 0
#x_pl <- seq(-10,10,by=0.1)
x1 <- seq(-1.5,2, length.out = 50)
x2 <- seq(-1.5,2, length.out = 50)

mapFeature <- function(x1,x2) {
    as.matrix(cbind(1,x1,x2,x1*x2,x1^2,x2^2))
}
#z <- matrix(NA,length(x1),length(x2))
#mapFeature(x1[i],x2[j]) %*% t(theta)

#z <- mapFeature(x1,x2) * as.vector(theta)


#y_pl <- uniroot(plot_func,c(0,10))
#op <- par()
#plot(subset(x[,2:3], y == 0), type="p",col="red")
#points(subset(x[,2:3], y == 1), type="p",col="black")
#contour(x1,x2,z=z,zlim=0,nlevels=0, add=T)
#lines(x_pl,plot_func(x_pl), type = "l")
