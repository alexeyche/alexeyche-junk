#!/usr/bin/RScript

require('R.matlab')

mat <- readMat('ex5/ex5data1.mat')

x.data <- mat$X
y <- mat$y
xval.data <- mat$Xval # for cv
yval <- mat$yval
xtest.data <- mat$Xtest
ytest <- mat$ytest

rm(mat)
#x <- x.data
m <- nrow(x.data) # count of measurement

mapFeature <- function(x, degree = 1) {
    if (degree <= 1) return(x)
    for(i in 2:degree) {
        x <- cbind(x, x[,1]^i)
    }
    return(x)
}

#x <- mapFeature(x)  # apply our mapFeature

#x <- cbind(rep(1,m),x)  # 1'th feature  
#xval <- cbind(rep(1,length(xval)),xval)  # 1'th feature  
#xtest <- cbind(rep(1,length(xtest)),xtest)  # 1'th feature  

#n <- ncol(x) # count of features


h <- function(x,theta) {
    tryCatch(x %*% t(theta) , error = function(e) { str(x); str(theta) } )
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

get_stats <- function(x, y, it, alpha) {
    stats <- null
    for(i in 2:m) {
        theta <- matrix(rep(1,n), nrow=1)
        x_temp <- x[1:i,]
        y_temp <- y[1:i,]
        theta <- grad.descent(x_temp, y_temp, theta, it, alpha, 0)
        
        stats <- rbind( stats, c( i, err(x_temp, y_temp, theta), err(xval, yval, theta) ) )
    }
    return(stats)
}

featureNormalize <- function(x) {
    means <- apply(x,2,mean)
    sds <- apply(x,2,sd)
    x <- t((t(x) - means )/ sds)
    return(x)
}

cookFeature <- function(x, degree) {
    x <- mapFeature(x, degree)
    x <- featureNormalize(x)
    x <- cbind(rep(1,nrow(x)), x)
}

x <- NULL
st <- NULL
yval <- featureNormalize(yval)
ytest <- featureNormalize(ytest)
y <- featureNormalize(y)

learn_curves <- function(maxpow = 10) {
    for( i in 1:maxpow ) {
        x <- cookFeature(x.data, i)
        xval <- cookFeature(xval.data, i)
        xtest <- cookFeature(xtest.data, i)
        n <- ncol(x)    
        st[[i]] <- get_stats(x, y, 5000, 0.002)

    }
}

x <- cookFeature(x.data, 4)
n <- ncol(x)

theta <- matrix(rep(1,n), nrow=1)
theta <- grad.descent(x, y, theta, 5000, 0.002, 0)

plot(featureNormalize(x.data), y, type="p", col="blue")
x.plot <- as.matrix(seq(-1.5,1.5, length.out = 300))
x.plot.d <- cookFeature(x.plot, 4)
y.plot <- h(x.plot.d, theta)
lines(x.plot, y.plot, col="red")
#save.image()
