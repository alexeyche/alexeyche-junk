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
#    print(x)
#    print(theta)
#    print('//-----------------')
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
    stats <- NULL
    for(i in 2:m) {
        theta <- matrix(rep(1,n), nrow=1)
        x_temp <- x[1:i,]
        y_temp <- y[1:i,]
#        str(x_temp)
#        str(y_temp)
#        str(theta)
#        print('//------------------------------------')
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
    x <- cbind(rep(1,m), x)
}

x <- NULL
st <- NULL
yval <- featureNormalize(yval)
ytest <- featureNormalize(ytest)
y <- featureNormalize(y)

for( i in 1:10 ) {
    x <- cookFeature(x.data, i)
    xval <- cookFeature(xval.data, i)
    xtest <- cookFeature(xtest.data, i)
    n <- ncol(x)    
    st[[i]] <- get_stats(x, y, 5000, 0.002)
}

par(mfrom = c(3,3))
for( i in 1:10 ) {
    data <- st[[i]]
    plot(data[,1],data[,3], ylim = c(0,max(data[,3])/3), type ="l" )
    #par(new=T)
    #plot(data[,1],data[,2])
}
#plot(stats[,c(1,3)], type="l", col="blue", ylim=c(0,200))
#lines(stats[,c(1,2)], col="green")
