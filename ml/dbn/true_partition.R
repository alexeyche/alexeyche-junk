#!/usr/bin/RScript

require(R.utils)

source('sys.R')
source('dbn_util.R')

c(num.vis, num.hid) := dim(model$W)

num_cases <- 2^num.hid

x <- c(0:(num_cases-1))

h <- int_to_bin(x)

log_prob <- h %*% t(model$hid_bias) +  sum.col(log(1+exp( h %*% t(model$W) + rep.row(model$vis_bias,num_cases) )) )
log_z_true <- logsum(log_prob)


