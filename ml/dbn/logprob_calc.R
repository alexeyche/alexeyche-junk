#!/usr/bin/RScript

require(R.utils)

source('sys.R')
source('dbn_util.R')


v <- data.all

log_prob <- function(v,log_z, model) {
    pd <- v %*% t(model$vis_bias) +  sum.col(log(1+exp( v %*% model$W + rep.row(model$hid_bias,nrow(v)) )) )
    log_prob <- sum(pd)/nrow(v) - log_z
    return(log_prob)
}

log_prob(data.all, log_z_est, model)