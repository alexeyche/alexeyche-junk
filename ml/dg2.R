#!/usr/bin/RScript

data <- read.csv("ex1/ex1data2.txt",sep=",",header=F)

n <- length(data)-1
m <- length(data$V1)

feat <- cbind(data$V1,data$V2,data$V3)
# normalize features

means <- apply(feat,2,mean)
sds <- apply(feat,2,sd)
feat <- t((t(feat) - means )/ sds)

# x and y

x <- cbind(feat[,1:n])
y <- cbind(feat[,n+1])

# 1 feature

x <- cbind(rep(1,m),x)
n <- n+1

h <- function (x_val,theta) {
    x_val %*% t(theta)
}

grad <- function (x, y, theta) {
    gradient <- (1/m) * (t(x) %*% ( h(x,theta) - y ))
    return(t(gradient))
}

grad.descent <- function (maxit, alpha) {
    theta <- matrix(rep(1,n), nrow=1)
    for( i in 1:maxit ) {
        g <- grad(x, y, theta)
        theta <- theta - alpha * g       
    }
    return(theta)
}

theta <- grad.descent(1000,0.01)

# analytical results with matrix algebra
solve(t(x)%*%x)%*%t(x)%*%y # w/o feature scaling
#solve(t(x.scaled)%*%x.scaled)%*%t(x.scaled)%*%y # w/ feature scaling

# results using canned lm function match results above
summary(lm(y ~ x[, 2] + x[, 3])) # w/o feature scaling
#summary(lm(y ~ x.scaled[, 2])) # w/feature scaling





