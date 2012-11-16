#!/usr/bin/RScript

data(iris)
test <- iris[c(48:50,98:100,148:150),]
iris <- iris[c(1:47,51:97,101:147),]
x.d <- iris[,1:4]
y.d <- iris[,5]
y <- levels(iris[,5])

x.test <- test[1:4]

n <- ncol(x.d)
m <- nrow(x.d)

source("../lib/featureCook.R")
source("../lib/parzen_window.h")

x.d <- featureNormalize(x.d)
test[,1:4] <- featureNormalize(test[,1:4])

 




test_nb <- function() {
    for(i in 1:nrow(test)) {
        h <- 0.4
        test_vector <- test[i,1:4]
        for( class in y) { 
            goal <- paste(test[i,5])
            py <- length(subset(y.d, y.d == class))/length(y.d)
            x.d_y <- as.matrix(subset(x.d, y.d == class))
            p <- parzen_window(test_vector,x.d_y, h)
            cat(class," in ", p*py, " is ",goal, "\n")
        }
        cat("//------------------------------------------------------------------------\n")
    }
}
