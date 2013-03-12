#!/usr/bin/RScript

source('sys.R')
source('makebatches.R')
source('dbn_util.R')


sigmoid <- function(x) {
  1/(1+exp(-x))
}

sample_bernoulli <- function(probs) {
  dims <- dim(probs)
  matrix(rbinom(prod(dims),size=1,prob=c(probs)),dims[1],dims[2])
}


prop_up <- function(v,u, model) {
  sigmoid( v %*% model$W + u %*% model$W_uh + rep.row(model$hid_bias, nrow(v)) )  
}

prop_down <- function(h,u,model) {
  sigmoid( h %*% t(model$W) + u %*% model$W_uv + rep.row(model$vis_bias, nrow(h)) )  
}

gibbs_hvh <- function(hid_probs,u,model) {    
  hid_states <- sample_bernoulli(hid_probs)
  vis_probs <- prop_down(hid_states,u, model)
  vis_states <- sample_bernoulli(vis_probs)    
  hid_probs <- prop_up(vis_states,u, model)
  
  list(vis_states = vis_states, vis_probs = vis_probs, hid_probs = hid_probs)
}

contrastive_divergence <- function(hid_probs, u, model, iter = 1) {    
  for(it in 1:iter) {
    # col for each hidden unit, row for each case
    c(vis_states, vis_probs, hid_probs) := gibbs_hvh(hid_probs,u, model)
  }
  return(list(vis_states.end = vis_states, vis_probs.end = vis_probs, hid_probs.end = hid_probs))
}

cross_entropy_cost <- function(input,fantasy) {
  mean(sum.row(input*log(fantasy)+(1-input)*log(1-fantasy)))
}

bias_cond <- function(u,model) {
  b_v <- model$vis_bias + u %*% model$W_uv
  b_h <- model$hid_bias + u %*% model$W_uh
  return(list(b_v, b_h))
}



num_vis <- 10
num_hid <- 10

num_cases <- 1000

data_all <- NULL
for(j in 1:(num_cases/10)) { 
  data <- matrix(0, nrow = 10, ncol=num_vis)  
  for(i in 1:num_vis) {
    data[i,i] <- 1  
  }
  data_all <- rbind(data_all, data)
}
n_delay <- 4
c(data_b, data_b_t) := makebatches(data = data_all, target.data = data_all, batch.size = 10, normalize=FALSE, permutate=FALSE)
num_batches <- dim(data_b)[3]
batch_start <- sample(seq(from=1,to=num_cases,by=n_delay))
# init model

train.params = list(e.w = 0.1, e.v = 0.001, e.h = 0.001, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 10000, cd.iter = 10, persistent = TRUE)  
num_hid <- 10

# Energy:
# E(v,h) = -b_v*v' -b_h*h' - v*W*h'


model <- list(W = array(0.01*rnorm(num_vis*num_hid),dim=c(num_vis,num_hid)), # visible units for row, hidden units for col
              W_uv = array(0.01*rnorm(num_vis*num_vis*n_delay),dim=c(num_vis,num_vis,n_delay)),
              W_uh = array(0.01*rnorm(num_vis*n_delay*num_hid),dim=c(num_vis,num_hid,n_delay)),
              vis_bias = array(0,dim = c(1,num_vis)), 
              hid_bias = array(0,dim = c(1,num_hid)),
              num_cases = num_cases)

train.params = list(e.w = 0.1, e.v = 0.1, e.h = 0.1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 50, cd.iter = 1)
for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])


W.inc <- hid_bias.inc <- vis_bias.inc <- 0

b <- 1


#for(i in 1:nrow(data)) {
  
  b_s <- batch_start[b]
  data.b[b_s]
  u <- matrix(t(data[i:n_delay,]), nrow=1)
  v <- matrix(data[i+n_delay,], nrow =1)
  
  hid_probs <- prop_up(v,u,model)
  c(vis_sample.fantasy, vis_probs.fantasy, hid_probs.fantasy) := contrastive_divergence(hid_probs,u,model,5)
  cost <- cross_entropy_cost(v,vis_probs.fantasy)
  
  c(b_v, b_h) := bias_cond(u, model)
  
  momentum <- 0.8
  W.inc <- momentum*W.inc + e.w*( ( t(v) %*% hid_probs - t(vis_sample.fantasy) %*% hid_probs.fantasy)/num_cases - w_cost*model$W)
  # d_log_p(v) / d_hid_bias_j
  hid_bias.inc <- momentum*hid_bias.inc + e.h*(sum.row(hid_probs) - sum.row(hid_probs.fantasy))/num_cases
  # d_log_p(v) / d_vis_bias_i
  vis_bias.inc <- momentum*vis_bias.inc + e.v*(sum.row(v) - sum.row(vis_sample.fantasy) - b_v)/num_cases
  
  
  model$W <- model$W + W.inc
  model$hid_bias <- model$hid_bias + hid_bias.inc
  model$vis_bias <- model$vis_bias + vis_bias.inc        
  uv_grad <- matrix(0, nrow=1, ncol=n_delay) 
  for(t in 1:n_delay) {
    uv_grad[,t] <- v - model$vis_vias - b_v
  }
#}

