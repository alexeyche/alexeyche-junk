#!/usr/bin/RScript

# http://sci2s.ugr.es/keel/dataset/data/classification/twonorm.zip
data <- read.csv("/home/alexeyche/prog/ml/datasets/twonorm.csv", header=F)
n <- ncol(data)-1
m <- nrow(data)

x <- data[,1:n]
y <- data[,n+1]
w <- matrix(0, nrow=1, ncol=n)
