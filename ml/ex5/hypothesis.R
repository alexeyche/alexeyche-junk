#!/usr/bin/RScript

h <- function(x,theta) {
    tryCatch(x %*% t(theta) , error = function(e) { print('error in hyp!'); str(x); str(theta) } )
}

err <- function(x, y, theta) {
    y.model <- h(x, theta)
    err <- sum((y - y.model)^2) / length(y)
    return(err)
}
