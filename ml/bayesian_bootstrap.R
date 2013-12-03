#!/usr/bin/RScript

library(gtools)

# Bayesian bootstrap
mean.bb <- function(x, n) {
  apply(rdirichlet(n, rep(1, length(x))), 1, weighted.mean, x = x)
}

# standard bootstrap
mean.fb <- function(x, n) {
  replicate(n, mean(sample(x, length(x), TRUE)))
}

set.seed(1)
reps <- 100000
data(cars)
x <- cars$dist
system.time(fbq <- quantile((mean.fb(x, reps)), c(0.025, 0.075)))
system.time(bbq <- quantile((mean.bb(x, reps)), c(0.025, 0.075)))

