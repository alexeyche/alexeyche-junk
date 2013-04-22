#!/usr/bin/RScript

neg_entropy <- function(x) {
    H.gauss <- function(var) {
        0.5*log(2*pi*exp(1)*(var))
    }
    d <- density(x, n =1000)
    p <- approx(d$x,d$y,x)$y
    H.x <- -mean(log(p))
    H.g <- H.gauss(var(X))
    H.g - H.x
}

excess_kurtosis <- function(x) {
    mean((x-mean(x))^4)/(var(x)^2) - 3
}


