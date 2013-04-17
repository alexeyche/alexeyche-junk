#!/usr/bin/RScript


source('sys.R')
source('dbn_util.R')


logsum <- function(x) {
    alpha <- max(x) - log(.Machine$double.xmax)/2
    alpha + log(sum(exp(x-alpha)))
}

logdiff <- function(x) {
    alpha <- max(x) - log(.Machine$double.xmax)/2
    alpha + log(diff(exp(x-alpha)))
}

calc_true_log_z <- function(model) {    
    c(num.vis, num.hid) := dim(model$W)    
    num_cases <- 2^num.hid    
    x <- c(0:(num_cases-1))    
    h <- int_to_bin(x)    
    log_prob <- h %*% t(model$hid_bias) +  sum.col(log(1+exp( h %*% t(model$W) + rep.row(model$vis_bias,num_cases) )) )
    
    log_z_true <- logsum(log_prob)
    return(log_z_true)
}

calc_log_probs <- function(data,model,log_z) {
    log_p_asterisk <- data %*% t(model$vis_bias) + sum.col(log(1+exp( data %*% model$W + rep.row(model$hid_bias,nrow(data)) )) )
    return(mean(log_p_asterisk) - log_z)
}
