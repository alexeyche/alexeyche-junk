#!/usr/bin/RScript
set.seed(3)

"gen" <- function(x){  
    return(3*x^2 + 2*x + 2 + rnorm(length(x))*0.5)  
} 

X <- runif(6)
Y <- gen(X)
Xcv <- runif(50)
Ycv <- gen(Xcv)

