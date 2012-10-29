#!/usr/bin/RScript

require('R.matlab')

mat <- readMat('ex5/ex5data1.mat')

x <- mat$X
y <- mat$y
xval <- mat$Xval # for cv
yval <- mat$yval
xtest <- mat$Xtest
ytest <- mat$ytest

rm(mat)

m <- nrow(x) # count of measurement

mapFeature <- function(x) {
    x1 <- x[,1]
    return(x1)
}

x <- mapFeature(x)  # apply our mapFeature

x <- cbind(rep(1,m),x)  # 1'th feature  
xval <- cbind(rep(1,length(xval)),xval)  # 1'th feature  
xtest <- cbind(rep(1,length(xtest)),xtest)  # 1'th feature  

n <- ncol(x) # count of features


theta <- matrix(rep(1,n), nrow=1)


h <- function(x,theta) {
    x %*% t(theta) 
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
#        cat("x_err: ", err(x, y, theta), ", x_cv_err: ",err(cbind(rep(1,m),xval),yval,theta),"x_test_err: ",err(cbind(rep(1,m),xtest),ytest,theta), "\n")

        theta <- theta - alpha * ( grad(x, y, theta) + reg * theta )   
    }
    return(theta)
}


#x.plot <- cbind(seq(-40,40,by=0.1))
#x.plot <- as.matrix(expand.grid(x.plot[,1],x.plot[,2]))
#y.plot <- h( cbind(1,mapFeature(x.plot)), theta )

#plot(x[,2],y)
#lines(x.plot,y.plot)

stats <- NULL

for(i in 2:m) {
    theta <- matrix(rep(1,n), nrow=1)
    x_temp <- x[1:i,]
    y_temp <- y[1:i,]
    theta <- grad.descent(x_temp, y_temp, theta, 5000, 0.002, 0)
    
    stats <- rbind( stats, c( i, err(x_temp, y_temp, theta), err(xval, yval, theta) ) )
}

plot(stats[,c(1,3)], type="l", col="blue", ylim=c(0,200))
lines(stats[,c(1,2)], col="green")
