#!/usr/bin/RScript

source('sys.R')
source('rbm.R')

prediction <- function(data, model, m = 50) {    
    start <- (model$n_delay+1)
    v <- matrix(data[start,], nrow=1)
    d <- array(0, dim=c(1, model$num_vis,model$n_delay+1))             
    d[,,1] <- v
    for(t in 1:model$n_delay) {
        d[,,t+1] <- data[start-t,]
    }
    bistar <- array(0, dim=c(1,model$num_vis))   
    bjstar <- array(0, dim=c(1,model$num_hid))     
    for(i in start:m) {        
        bistar[,] <- bjstar[,] <- 0
        for (t in 1:n_delay) {            
            bistar = bistar + d[,,t+1] %*% model$W_uv[,,t]
            bjstar = bjstar + d[,,t+1] %*% model$W_uh[,,t]
        }    
        # prop up   
        v <- array(abs(0.001*rnorm(model$num_vis)), dim=c(1,model$num_vis))
        for(it in 1:100) {
            hid_probs <- sigmoid( v %*% model$W + rep.row(model$hid_bias, nrow(v))+bjstar)  
            hid_states <- sample_bernoulli(hid_probs)
            vis_probs <- sigmoid( hid_states %*% t(model$W) + rep.row(model$vis_bias, nrow(hid_probs))+bistar)  
            v <- vis_probs            
        }
        for(del in model$n_delay:2) {
            d[,,del+1] <- d[,,del]           
        }
        d[,,2] <- sample_bernoulli(v)
        printf("%d ",d[,,2])
        cat("\n")
    }
}

gen_data <- function(n, model) {
    data_all <- NULL
    for(j in 1:(n/6)) { 
      data <- matrix(0, nrow = 6, ncol=model$num_vis)  
      for(i in 1:(model$num_vis-4)) {
        data[i,i] <- 1  
        data[i,(i+2)] <- 1  
        data[i,(i+4)] <- 1  
      }
      data_all <- rbind(data_all, data)
    }
    x <- 6
    tt <- (model$n_delay - model$n_delay %% 6)/6 
    for(t in 1:(tt+1)) {
        if(t == (tt+1)) {
            x <- model$n_delay %% 6
        }        
        data <- matrix(0, nrow = x, ncol=model$num_vis)  
        for(i in 1:x) {
            data[i,i] <- 1  
            data[i,(i+2)] <- 1  
            data[i,(i+4)] <- 1  
        }
        data_all <- rbind(data_all, data)
    }
    
    return(data_all)
}

sigmoid <- function(x) {
    1/(1+exp(-x))
}

prop_up <- function(v, bjstar, model) {
    sigmoid( v %*% model$W + rep.row(model$hid_bias, nrow(v)) + bjstar )
}

prop_down <- function(h, bistar, model) {
    sigmoid( h %*% t(model$W) + rep.row(model$vis_bias, nrow(h)) + bistar )
}

n_delay <- 3
num_vis <- 10
num_hid <- 15
num_cases <- 1200
batch_size <- 100


model <- list(W = array(0.01*rnorm(num_vis*num_hid),dim=c(num_vis,num_hid)), # visible units for row, hidden units for col
              W_uv = array(0.01*rnorm(num_vis*num_vis*n_delay),dim=c(num_vis,num_vis,n_delay)),
              W_uh = array(0.01*rnorm(num_vis*n_delay*num_hid),dim=c(num_vis,num_hid,n_delay)),
              vis_bias = array(0,dim = c(1,num_vis)), 
              hid_bias = array(0,dim = c(1,num_hid)),
              num_cases = num_cases,
              n_delay = n_delay,
              num_vis = num_vis,
              num_hid = num_hid)

train.params = list(e.w =  0.01, e.v =  0.01, e.h =  0.01, w_decay = 0.0002, e.w_uv =  0.01, e.w_uh =  0.01,
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 1000, cd.iter = 1)
for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])

data_all <- gen_data(num_cases, model)
batch_start <- sample((n_delay+1):(num_cases+n_delay))
num_batches <- num_cases/batch_size

# vars
W_inc <-  vis_bias_inc <- hid_bias_inc <- 0
W_uv_inc <- array(0, dim=c(num_vis,num_vis,n_delay))
W_uh_inc <- array(0, dim=c(num_vis,num_hid,n_delay))
pos_W_uv_grad <- array(0, dim=c(model$num_vis, model$num_vis, model$n_delay))
pos_W_uh_grad <- array(0, dim=c(model$num_vis, model$num_hid, model$n_delay))
neg_W_uv_grad <- array(0, dim=c(model$num_vis, model$num_vis, model$n_delay))
neg_W_uh_grad <- array(0, dim=c(model$num_vis, model$num_hid, model$n_delay))
# loop
for(epoch in 1:epochs) {
    for(b in 1:num_batches) {
        
        b_s <- batch_start[((b-1)*batch_size+1):((b-1)*batch_size+batch_size)]
        
        data <- array(dim=c(batch_size, num_vis,n_delay+1))
        
        data[,,1] <- data_all[b_s,]
        for(t in 1:model$n_delay) {
            data[,,t+1] <- data_all[b_s-t,]
        }
        
        # calc bias conditional contributions
        bistar <- array(0, dim=c(batch_size, model$num_vis))
        bjstar <- array(0, dim=c(batch_size, model$num_hid))
        for (t in 1:model$n_delay) {
            bistar = bistar + data[,,t+1] %*% model$W_uv[,,t]
            bjstar = bjstar + data[,,t+1] %*% model$W_uh[,,t]
        }
        
        v <- data[,,1]
        hid_probs <- prop_up(v, bjstar, model)
        hid_states <- sample_bernoulli(hid_probs)
        
        pos_W_grad <- t(v) %*% hid_states
        pos_vb_grad <- sum.row(v - rep.row(model$vis_bias, batch_size) - bistar)
        pos_hb_grad <- sum.row(hid_states)
        for(t in 1:model$n_delay) {
            pos_W_uv_grad[,,t] <- t(data[,,t+1]) %*% (v - rep.row(model$vis_bias, batch_size) - bistar) 
            pos_W_uh_grad[,,t] <- t(data[,,t+1]) %*% (hid_states)
        }
        
        vis_probs.fant <- prop_down(hid_states, bistar, model)
        hid_probs.fant <- prop_up(vis_probs.fant, bjstar, model)
        
        neg_W_grad <- t(vis_probs.fant) %*% hid_probs.fant
        neg_vb_grad <- sum.row(vis_probs.fant - rep.row(model$vis_bias, batch_size) - bistar)
        neg_hb_grad <- sum.row(hid_probs.fant)
        for(t in 1:model$n_delay) {
            neg_W_uv_grad[,,t] <- t(data[,,t+1]) %*% (vis_probs.fant - rep.row(model$vis_bias, batch_size) - bistar) 
            neg_W_uh_grad[,,t] <- t(data[,,t+1]) %*% (hid_probs.fant)
        }
        cost <- sum((v -vis_probs.fant)^2)
        cat("Epoch #",epoch, ", cost: ", cost, "\n")
        
        # momentum
        momentum <- fin.moment
        if(epoch<=5) {
            momentum <- 0
        }
        # calc increments
        W_inc <- momentum * W_inc + e.w * ((pos_W_grad - neg_W_grad)/num_cases - w_decay*model$W)
        vis_bias_inc <- momentum * vis_bias_inc + e.h * ((pos_vb_grad - neg_vb_grad)/num_cases)    
        hid_bias_inc <- momentum * hid_bias_inc + e.h * ((pos_hb_grad - neg_hb_grad)/num_cases)
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
        