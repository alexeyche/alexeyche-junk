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
num_hid <- 12 

num_cases <- 1000

data_all <- NULL
for(j in 1:(num_cases/10)) { 
  data <- matrix(0, nrow = 10, ncol=num_vis)  
  for(i in 1:num_vis) {
    data[i,i] <- 1  
  }
  data_all <- rbind(data_all, data)
}


# init model

train.params = list(e.w = 0.1, e.v = 0.001, e.h = 0.001, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 10, cd.iter = 10, persistent = TRUE)  

# Energy:
# E(v,h) = -b_v*v' -b_h*h' - v*W*h'

n_delay <- 3
model <- list(W = array(0.01*rnorm(num_vis*num_hid),dim=c(num_vis,num_hid)), # visible units for row, hidden units for col
              W_uv = array(0.01*rnorm(num_vis*num_vis*n_delay),dim=c(num_vis,num_vis,n_delay)),
              W_uh = array(0.01*rnorm(num_vis*n_delay*num_hid),dim=c(num_vis,num_hid,n_delay)),
              vis_bias = array(0,dim = c(1,num_vis)), 
              hid_bias = array(0,dim = c(1,num_hid)),
              num_cases = num_cases)

train.params = list(e.w = 0.1, e.v = 0.1, e.h = 0.1, w_decay = 0.0002, e.w_uv = 0.1, e.w_uh = 0.1,
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 500, cd.iter = 1)
for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])

batch_start <- sample((n_delay+1):num_cases)
batch_size <- 100
num_batches <- nrow(data_all)/batch_size

W_inc <- hid_bias_inc <- vis_bias_inc <- 0
W_uv_inc <- array(0,dim=c(num_vis,num_vis,n_delay))
W_uh_inc <- array(0,dim=c(num_vis,num_hid,n_delay))

num_cases <- batch_size

for(epoch in 1:epochs) {
    for(b in 1:num_batches) {
        if (((b-1)*batch_size+batch_size) > length(batch_start)) {
            break
        } else {
            b_s <- batch_start[((b-1)*batch_size+1):((b-1)*batch_size+batch_size)]
        }
        num_cases <- length(b_s)
        data <- array(dim=c(num_cases, num_vis,n_delay+1))
        data[,,1] <- data_all[b_s,]
        for(t in 1:n_delay) {
            data[,,t+1] <- data_all[b_s-t,]
        }
        
        # calc bias conditional contributions
        bistar <- array(0, dim=c(num_cases,num_vis))   
        bjstar <- array(0, dim=c(num_cases,num_hid))     
        for (t in 1:n_delay) {
            bistar = bistar + data[,,t+1] %*% model$W_uv[,,t]
            bjstar = bjstar + data[,,t+1] %*% model$W_uh[,,t]
        }
        # prop up   
        v <- data[,,1]
        hid_probs <- sigmoid( v %*% model$W + rep.row(model$hid_bias, nrow(v)) + bjstar)  
        hid_state <- sample_bernoulli(hid_probs)
        
        # calc positive grads
        pos_W_grad <- t(v) %*% hid_state
        pos_vis_bias_grad <- sum.row(v-rep.row(model$vis_bias,num_cases)-bistar)
        pos_hid_bias_grad <- sum.row(hid_state)
        pos_W_uv_grad <- array(0, dim=c(num_vis,num_vis,n_delay))
        pos_W_uh_grad <- array(0, dim=c(num_vis,num_hid,n_delay))
        for (t in 1:n_delay) {
            pos_W_uv_grad[,,t] <- t(data[,,1] - rep.row(model$vis_bias, num_cases) - bistar) %*% data[,,t+1]
            pos_W_uh_grad[,,t] <- t(hid_state) %*% data[,,t+1]
        }
        # prop down
        h <- hid_state
        vis_probs <- sigmoid( h %*% t(model$W) + rep.row(model$vis_bias, nrow(h)) + bistar)  
        # and up again
        hid_probs <- sigmoid( vis_probs %*% model$W + rep.row(model$hid_bias, nrow(vis_probs)) + bjstar)  
        
        # calc negative grads
        neg_W_grad <- t(vis_probs) %*% hid_probs
        neg_vis_bias_grad <- sum.row(vis_probs-rep.row(model$vis_bias, num_cases)-bistar)
        neg_hid_bias_grad <- sum.row(hid_probs)
        neg_W_uv_grad <- array(0, dim=c(num_vis,num_vis,n_delay))
        neg_W_uh_grad <- array(0, dim=c(num_vis,num_hid,n_delay))
        for (t in 1:n_delay) {
            neg_W_uv_grad[,,t] <- t(vis_probs - rep.row(model$vis_bias, num_cases) - bistar) %*% data[,,t+1]
            neg_W_uh_grad[,,t] <- t(hid_probs) %*% data[,,t+1] 
        }
        # calc recontruction error
        cost <- sum((v -vis_probs)^2)
        cat("Epoch #",epoch, ", cost: ", cost,"\n")
        # momentum
        momentum <- 0.9
        if(epoch<=5) {
            momentum <- 0
        }
        # calc increments
        W_inc <- momentum * W_inc + e.w * ((pos_W_grad - neg_W_grad)/num_cases - w_decay*model$W)
        vis_bias_inc <- momentum * vis_bias_inc + e.h * ((pos_vis_bias_grad - neg_vis_bias_grad)/num_cases)    
        hid_bias_inc <- momentum * hid_bias_inc + e.h * ((pos_hid_bias_grad - neg_hid_bias_grad)/num_cases)
        for(t in 1:n_delay) {
            W_uv_inc[,,t] <- momentum * W_uv_inc[,,t] + e.w_uv * ((pos_W_uv_grad[,,t]-neg_W_uv_grad[,,t])/num_cases - w_decay*model$W_uv[,,t])
            W_uh_inc[,,t] <- momentum * W_uh_inc[,,t] + e.w_uh * ((pos_W_uh_grad[,,t]-neg_W_uh_grad[,,t])/num_cases - w_decay*model$W_uh[,,t])
        }
        model$W <- model$W + W_inc
        model$vis_bias <- model$vis_bias + vis_bias_inc
        model$hid_bias <- model$hid_bias + hid_bias_inc
        for(t in 1:n_delay){
            model$W_uv[,,t] <- model$W_uv[,,t] + W_uv_inc[,,t]
            model$W_uh[,,t] <- model$W_uh[,,t] + W_uh_inc[,,t]
        }
    }
}

