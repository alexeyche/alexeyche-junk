#!/usr/bin/RScript

source('sys.R')
source('rbm.R')

betas <- c(seq(0,0.5,by=1e-03), seq(0.5,0.9,by=1e-04), seq(0.9,1,by=1e-05))
numruns <- 100
c(num.vis, num.hid) := dim(model$W)

log_base_rate <- log(colMeans(data.all)) - log(1-colMeans(data.all))

# init base_model
base_model <- list(W = array(0,dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
              vis_bias = array(log_base_rate, dim=c(1,num.vis)), 
              hid_bias = array(0,dim = c(1,num.hid)),
              num.cases = numruns)


logsum <- function(x) {
    alpha <- max(x) - log(.Machine$double.xmax)/2
    alpha + log(sum(exp(x-alpha)))
}

prop_up.t <- function(v,bb,model) {
    sigmoid( bb * (v %*% model$W + rep.row(model$hid_bias,nrow(v)) ) ) 
}

prop_down.t <- function(h, bb, base_model, model) {
    sigmoid( (1-bb) * rep.row(base_model$vis_bias, nrow(h)) + bb*(h %*% t(model$W) + rep.row(model$vis_bias,nrow(h)) ) )
}

free_energy.t <- function(v,bb,model) {    
    - bb*(v %*% t(model$vis_bias)) - sum.col(log(1+exp(bb * (v %*% model$W + rep.row(model$hid_bias,nrow(v)) )) ) ) 
}

free_energy.t.base <- function(v,bb,model) {
    - bb*(v %*% t(model$vis_bias))    
}

log_p_k <- function(v, bb, base_model, model) {
    -free_energy.t.base(v,(1-bb),base_model) -free_energy.t(v,bb,model)
}

# init first logw
bb <- betas[1]
i <- 1
v.m <- rep.row(sigmoid(base_model$vis_bias), numruns)
v <- sample_bernoulli(v.m)
logw <- log_p_k(v, bb, base_model,model)

for( bb_i in seq(2, length(betas)-1) ) {
    bb <- betas[bb_i]
    i <- i+1
    logw <- logw + log_p_k(v, bb, base_model,model)
    if(i %% 500 == 0) {
        cat("var log_w: ", var(logw), " (", i/length(betas),")\n")
    }
    h.m <- prop_up.t(v, bb, model)
    h <- sample_bernoulli(h.m)
    v.m <- prop_down.t(h, bb, base_model, model)
    v <- sample_bernoulli(v.m)
    logw <- logw - log_p_k(v, bb, base_model, model)    
}
logw <- logw + log_p_k(v, 1, base_model,model)

r <- logsum(logw) - log(numruns)
logstd_ais <- log(apply(exp(logw-mean(logw)),2,sd)) + mean(logw) - log(numruns)/2

log_z_base <- sum(log(1+exp(base_model$vis_bias))) + (num.hid)*log(2)
log_z_est <- r + log_z_base
