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

#normalize
for(i in 1:n) {
    x.d[,i] <- (x.d[,i] - mean(x.d[,i]))/sd(x.d[,i])
    test[,i] <- (test[,i] - mean(test[,i]))/sd(test[,i])
}

# vector form
K <- function(r) {
  ignore <- abs(r) > 1
  r <- 3/4 * (1-r^2)
  r[ignore] <- 0
  return(r)
}

# x     - x1  x2  .. xN
# x.all - x11 x12 .. x1N  - feature matrix
#         x21 x22 .. x2N
#         ...
#         xM1 xM2 .. xMN  
# h - window
parzen_window <- function(x, x.all, h) {
  kernels_pr <- 1
  n <- ncol(x.all)
  m <- nrow(x.all)  
  # it is product on j
  for(j in 1:n) {
    kernels_pr <- kernels_pr * K(abs(x.all[,j] - x[,j])/h)/h
  }
  # it is sum on i
  sum(kernels_pr)/m
}

eval_parzen_window <- function(x.all, h) {
    p <- NULL
    for(i in 1:nrow(x.all)) {
        x.i <- matrix(x.all[i,], nrow=1)
        p <- rbind(p, parzen_window(x.i, x.all, h))
    }
    return(p)
}

test_parzen_window <- function() {
    library(scatterplot3d)
    x.t <- as.matrix( cbind(rnorm(2000),rnorm(2000)) )
    par(mfrow=c(3,3))
    for(i in 1:9) {
       p <- eval_parzen_window(x.t, 0.3)
        scatterplot3d(x.t[,1],x.t[,2], p)
    }
}
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
        cat("//------------------\n")
    }
}

#    
#
#    cat("versicolor in ", p2*py, " is ", goal, "\n")
#    cat("virginica in ", p3*py, " is ", goal, "\n")
#}
