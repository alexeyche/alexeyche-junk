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

free_energy <- function(data,model) {    
    v <- data[,,1]
    m <- nrow(v)
    bistar <- array(0, dim=c(m,model$num_vis))   
    bjstar <- array(0, dim=c(m,model$num_hid))     
    for (t in 1:model$n_delay) {            
        bistar = bistar + data[,,t+1] %*% model$W_uv[,,t]
        bjstar = bjstar + data[,,t+1] %*% model$W_uh[,,t]
    }     
    - (v+bistar) %*% t(model$vis_bias) - sum.col(log(1+exp(v %*% model$W + rep.row(model$hid_bias,m) + bjstar )) )
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

prediction <- function(data, model) {
    m <- 30
    v <- matrix(data[4,], nrow=1)
    for(i in (model$n_delay+1):m) {
        d <- array(0, dim=c(1, model$num_vis,model$n_delay+1))             
        for(t in 1:model$n_delay) {
            d[,,t+1] <- data[i-t,]
        }
        bistar <- array(0, dim=c(1,model$num_vis))   
        bjstar <- array(0, dim=c(1,model$num_hid))     
        for (t in 1:n_delay) {            
            bistar = bistar + d[,,t+1] %*% model$W_uv[,,t]
            bjstar = bjstar + d[,,t+1] %*% model$W_uh[,,t]
        }    
        # prop up   
        #v <- data[,,1]  #array(abs(0.001*rnorm(model$num_vis)), dim=c(1,model$num_vis))
        for(it in 1:30) {
            hid_probs <- sigmoid( v %*% model$W + rep.row(model$hid_bias, nrow(v))+bjstar)  
            hid_states <- sample_bernoulli(hid_probs)
            vis_probs <- sigmoid( hid_states %*% t(model$W) + rep.row(model$vis_bias, nrow(hid_probs))+bistar)  
            v <- vis_probs            
        }
        cat(sample_bernoulli(vis_probs), "\n")
    }
}


num_vis <- 10
num_hid <- 50

num_cases <- 1200

data_all <- NULL
for(j in 1:(num_cases/6)) { 
  data <- matrix(0, nrow = 6, ncol=num_vis)  
  for(i in 1:(num_vis-4)) {
    data[i,i] <- 1  
    data[i,i+2] <- 1
    data[i,i+4] <- 1
  }
  data_all <- rbind(data_all, data)
}
n_delay <- 3

data <- matrix(0, nrow = n_delay, ncol=num_vis)  
for(i in 1:n_delay) {
    data[i,i] <- 1  
    data[i,i+2] <- 1
    data[i,i+4] <- 1
}
data_all <- rbind(data_all, data)

# init model

# Energy:
# E(v,h) = -b_v*v' -b_h*h' - v*W*h'


model <- list(W = array(0.01*rnorm(num_vis*num_hid),dim=c(num_hid,num_vis)), # visible units for row, hidden units for col
              A = array(0.01*rnorm(num_vis*num_vis*n_delay),dim=c(num_vis,num_vis,n_delay)),
              B = array(0.01*rnorm(num_vis*n_delay*num_hid),dim=c(num_hid,num_vis,n_delay)),
              vis_bias = array(0,dim = c(1,num_vis)), 
              hid_bias = array(0,dim = c(1,num_hid)),
              num_cases = num_cases,
              n_delay = n_delay,
              num_vis = num_vis,
              num_hid = num_hid)

train.params = list(e.w =  0.001, e.v =  0.001, e.h =  0.001, w_decay = 0.0002, e.w_uv =  0.001, e.w_uh =  0.001,
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 3000, cd.iter = 1)
for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])

batch_start <- sample((n_delay+1):(num_cases+n_delay))
batch_size <- 100
num_batches <- (nrow(data_all)-n_delay)/batch_size

W_inc <- hid_bias_inc <- vis_bias_inc <- 0
A_inc <- array(0,dim=c(num_vis,num_vis,n_delay))
B_inc <- array(0,dim=c(num_hid,num_vis,n_delay))

num_cases <- batch_size

for(epoch in 1:epochs) {
    for(b in 1:num_batches) {
        b_s <- batch_start[((b-1)*batch_size+1):((b-1)*batch_size+batch_size)]
        
        num_cases <- length(b_s)
        data <- array(dim=c(num_cases, num_vis,n_delay+1))
        data[,,1] <- data_all[b_s,]
        for(t in 1:n_delay) {
            data[,,t+1] <- data_all[b_s-t,]
        }
        
        # calc bias conditional contributions
        bistar <- array(0, dim=c(model$num_vis, num_cases))   
        bjstar <- array(0, dim=c(model$num_hid, num_cases))     
        for (t in 1:n_delay) {
            bistar = bistar + model$A[,,t] %*% t(data[,,t+1]) 
            bjstar = bjstar + model$B[,,t] %*% t(data[,,t+1])
        }
        # prop up   
        v <- data[,,1]
        hid_probs <- sigmoid( model$W %*% t(v) + t(rep.row(model$hid_bias, num_cases)) + bjstar)  
        hid_state <- sample_bernoulli(hid_probs)
        
        # calc positive grads
        pos_W_grad <- t(hid_state) %*% v
        pos_vis_bias_grad <- sum.row(t(v)-t(rep.row(model$vis_bias,num_cases))-bistar)
        pos_hid_bias_grad <- sum.row(hid_state)
        pos_A_grad <- array(0, dim=c(num_vis,num_vis,n_delay))
        pos_B_grad <- array(0, dim=c(num_hid,num_vis,n_delay))
        for (t in 1:n_delay) {
            pos_A_grad[,,t] <- (t(v) - t(rep.row(model$vis_bias, num_cases)) - bistar) %*% data[,,t+1]
            pos_B_grad[,,t] <- t(hid_state) %*% data[,,t+1]
        }
        # prop down
        h <- hid_state
        vis_probs <- sigmoid( h %*% model$W + rep.row(model$vis_bias, num_cases) + t(bistar))  
        # and up again
        hid_probs <- sigmoid( model$W %*% t(vis_probs) + t(rep.row(model$hid_bias, num_cases)) + bjstar)  
        
        # calc negative grads
        neg_W_grad <- hid_probs %*% vis_probs
        neg_vis_bias_grad <- sum.row(t(vis_probs)-t(rep.row(model$vis_bias, num_cases))-bistar)
        neg_hid_bias_grad <- sum.row(hid_probs)
        neg_W_uv_grad <- array(0, dim=c(num_vis,num_vis,n_delay))
        neg_W_uh_grad <- array(0, dim=c(num_hid,num_vis,n_delay))
        for (t in 1:n_delay) {
            neg_W_uv_grad[,,t] <- (t(vis_probs) - rep.row(model$vis_bias, num_cases) - bistar) %*% data[,,t+1]
            neg_W_uh_grad[,,t] <- hid_probs %*% data[,,t+1] 
        }
        # calc recontruction error
        cost <- sum((v -vis_probs)^2)
        f <- free_energy(data,model)
        cat("Epoch #",epoch, ", cost: ", cost, " free energy: ", mean(f), "\n")
        # momentum
        momentum <- fin.moment
        if(epoch<=5) {
            momentum <- 0
        }
        # calc increments
        W_inc <- momentum * W_inc + e.w * ((pos_W_grad - neg_W_grad)/num_cases - w_decay*model$W)
        vis_bias_inc <- momentum * vis_bias_inc + e.h * ((pos_vis_bias_grad - neg_vis_bias_grad)/num_cases)    
        hid_bias_inc <- momentum * hid_bias_inc + e.h * ((pos_hid_bias_grad - neg_hid_bias_grad)/num_cases)
        for(t in 1:n_delay) {
            A_inc[,,t] <- momentum * A_inc[,,t] + e.w_uv * ((pos_A_grad[,,t]-neg_A_grad[,,t])/num_cases - w_decay*model$A[,,t])
            B_inc[,,t] <- momentum * B_inc[,,t] + e.w_uh * ((pos_B_grad[,,t]-neg_B_grad[,,t])/num_cases - w_decay*model$B[,,t])
        }
        model$W <- model$W + W_inc
        model$vis_bias <- model$vis_bias + vis_bias_inc
        model$hid_bias <- model$hid_bias + hid_bias_inc
        for(t in 1:n_delay){
            model$A[,,t] <- model$A[,,t] + A_inc[,,t]
            model$B[,,t] <- model$B[,,t] + B_inc[,,t]
        }
    }
}

