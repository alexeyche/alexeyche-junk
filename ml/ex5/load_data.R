#!/usr/bin/RScript

require('R.matlab')

mat <- readMat('ex5data1.mat')
xdata <- mat$X
ydata <- mat$y
xval <- mat$Xval # for cv
yval <- mat$yval
xtest <- mat$Xtest
ytest <- mat$ytest

source('featureCook.R')

x.n <- featureNormalize(xdata)
y.n <- featureNormalize(ydata)
xval.n <- featureNormalize(xval)
yval.n <- featureNormalize(yval)
xtest.n <- featureNormalize(xtest)
ytest.n <- featureNormalize(ytest)

rm(mat)

test <- function() { 
    par(mfrow=c(1,2))
    plot(x.n, y.n, col='green')
    points(xval.n, yval.n, col='blue')
    points(xtest.n, ytest.n, col='red')
    plot(xdata, ydata, col='green')
    points(xval, yval, col='blue')
    points(xtest, ytest, col='red')
}

