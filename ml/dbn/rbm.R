#!/usr/bin/RScript

require(reshape2)
require(ggplot2)


# Energy:
# E(v,h) = -b'v-c'h-h'Wv
energy_vector <- function(v,h,model) {
    - v %*% t(model$vis_bias) - h %*% t(model$hid_bias) - v %*% model$W %*% t(h)
}

energy_all <- function(v,h,model) {
    E <- NULL
    for(case in 1:nrow(v)) {        
        E <- rbind(E, energy_vector(t(v[case,]), t(h[case,]),model))
    }
    return(E)
}

cross_entropy_cost <- function(input,fantasy) {
    mean(sum.row(input*log(fantasy)+(1-input)*log(1-fantasy)))
}


sigmoid <- function(x) {
    1/(1+exp(-x))
}

sample_bernoulli <- function(probs) {
    dims <- dim(probs)
    matrix(rbinom(prod(dims),size=1,prob=c(probs)),dims[1],dims[2])
}

free_energy <- function(v,model) {    
    - v %*% t(model$vis_bias) - sum.col(log(1+exp(v %*% model$W + rep.row(model$hid_bias,nrow(v)) )) )
}

prop_up <- function(given_v, model) {    
    if(model$type == 'BB') {
        out <- sigmoid( given_v %*% model$W + rep.row(model$hid_bias, nrow(given_v)) )
    } else
    if(model$type == 'BG') {
        out <- given_v %*% model$W + rep.row(model$hid_bias, nrow(given_v))
    } else 
    if(model$type == 'GB') {
        out <- sigmoid( (given_v %*% model$W)/model$sigma + rep.row(model$hid_bias, nrow(given_v)) )
    } else {
        cat("Unknown model type")
    }
    return(out)
}

prop_down <- function(given_h, model) {
    if(model$type == 'BB') {
        out <- sigmoid( given_h %*% t(model$W) + rep.row(model$vis_bias, nrow(given_h)) )    
    } else
    if(model$type == 'BG') {
        out <- sigmoid( given_h %*% t(model$W) + rep.row(model$vis_bias, nrow(given_h)) )    
    } else 
    if(model$type == 'GB') {
        out <- model$sigma * given_h %*% t(model$W) + rep.row(model$vis_bias, nrow(given_h))
    } else {
        cat("Unknown model type")
    }
    return(out)
}

gibbs_hvh <- function(hid_probs,model) {    
    if ((model$type == 'GB' ) | (model$type == 'BB')) {
        hid_states <- sample_bernoulli(hid_probs)
    } else 
    if (model$type == 'BG') {
        hid_states <- hid_probs
    }
    # p(v_model|h) calculate visible state with given hidden units state from positive phase    
    vis_probs <- prop_down(hid_states, model)
    
    if ((model$type == 'BG' ) | (model$type == 'BB')) {
        vis_states <- sample_bernoulli(vis_probs)
    } else 
    if (model$type == 'GB') {
        vis_states <- vis_probs
    }
    # p(h|v_model) calculate hidden state with given state of visible units    
    hid_probs <- prop_up(vis_states, model)
    
    list(vis_states = vis_states, vis_probs = vis_probs, hid_probs = hid_probs)
}
gibbs_vhv <- function(vis_probs,model) {    
    if ((model$type == 'BG' ) | (model$type == 'BB')) {
        vis_states <- sample_bernoulli(vis_probs)
    } else 
    if (model$type == 'GB') {
        vis_states <- vis_probs
    }
    # p(h|v_model) calculate hidden state with given state of visible units    
    hid_probs <- prop_up(vis_states, model)
    
    if ((model$type == 'GB' ) | (model$type == 'BB')) {
        hid_states <- sample_bernoulli(hid_probs)
    } else 
    if (model$type == 'BG') {
        hid_states <- hid_probs
    }
    # p(v_model|h) calculate visible state with given hidden units state from positive phase    
    vis_probs <- prop_down(hid_states, model)
    
    return(vis_probs)
}

contrastive_divergence <- function(hid_probs, model, iter = 1) {    
    for(it in 1:iter) {
         # col for each hidden unit, row for each case
        c(vis_states, vis_probs, hid_probs) := gibbs_hvh(hid_probs, model)
    }
    return(list(vis_states.end = vis_states, vis_probs.end = vis_probs, hid_probs.end = hid_probs))
}

# as it done in deeplearning.net
pseudo_likelihood <- function(v,model) {
    num.vis <- ncol(v)
    i <- sample(1:num.vis,1)
    v_inv <- v
    v_inv[,i] <- 1- v_inv[,i] 
    mean(num.vis*log(sigmoid((free_energy(v_inv,model) - free_energy(v,model)))))
}

collect_hidden_statistics <- function(model, batched.data) {
    c(num.cases, num.vis, num.batches) := dim(batched.data)
    num.hid <- ncol(model$W)
    batched.hid_probs <- array(0, dim = c(num.cases, num.hid, num.batches))
    for(batch in 1:num.batches) {
        data <- batched.data[,,batch]
        batched.hid_probs[,,batch] <- prop_up(data,model)
    }
    return(batched.hid_probs)
}

train_rbm <- function(batched.data, params, num.hid, type = 'BB', sigma = 1, model = NULL) {
    c(num.cases, num.vis, num.batches) := dim(batched.data)
    
    
    for (v in 1:length(params)) assign(names(params)[v], params[[v]])
    
    if(is.null(model)) {  # need to create new model
        
        model <- list(W = array(0.01*rnorm(num.vis*num.hid),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
                      vis_bias = array(0,dim = c(1,num.vis)), 
                      hid_bias = array(0,dim = c(1,num.hid)),
                      num.cases = num.cases, type = type)
        if ((type == 'GB') | (type == 'BG')) {
            model <- c(model, sigma = sigma )    
        }
        epoch <- 1        
    } else {
        if(is.null(num.hid)) {
            cat("Need specified hidden units, or model\n")
        }
    }
    if(persistent) {
        persist.hid_probs <- matrix(0, nrow = num.cases, ncol = num.hid)
    }
    #png(filename=sprintf("0_epoch_%d",epoch,num.vis))        
    #hist(model$W)
    #dev.off()
    cd.upd <- 0
    W.inc <- hid_bias.inc <- vis_bias.inc <- 0
    for(epoch in 1:epochs) {               
        for(batch in 1:num.batches) {
            # positive part, given data                                
            data <- batched.data[,,batch]
            hid_probs <- prop_up(data,model) # v*W + bias_h    
            if((epoch/epochs > 0.5)&(cd.upd == 0)) {
                cd.iter <- cd.iter + 1
                cd.upd <- cd.upd + 1
            }
            if((epoch/epochs > 0.75)&(cd.upd == 1)) {
                cd.iter <- cd.iter + 1
                cd.upd <- cd.upd + 1
            }
            if((epoch/epochs > 0.85)&(cd.upd == 2)) {
                cd.iter <- cd.iter + 3
                cd.upd <- cd.upd + 1
            }
            # collecting negative samples
            if(persistent) {
                c(vis_sample.fantasy, vis_probs.fantasy, hid_probs.fantasy) := contrastive_divergence(persist.hid_probs, model, cd.iter)
                persist.hid_probs = hid_probs.fantasy
            } else {    
                c(vis_sample.fantasy, vis_probs.fantasy, hid_probs.fantasy) := contrastive_divergence(hid_probs,model, cd.iter)
            }
            cost <- cross_entropy_cost(data,vis_probs.fantasy)        
            pseudo.lh <- pseudo_likelihood(data,model)
            E.free.mean <- sum(free_energy(data, model))/num.cases
            cat("Epoch # ", epoch, "cost: ", cost," free energy: ", E.free.mean, "pseudo lh: ",pseudo.lh,"\n") 
            # moment stuff 
            momentum <- fin.moment
            if (epoch <= 5) {
                momentum <- init.moment
            }
            # deravatives over log p(v)
            # d_log_p(v) / d_W_ij
            W.inc <- momentum*W.inc + e.w*( ( t(data) %*% hid_probs - t(vis_sample.fantasy) %*% hid_probs.fantasy)/num.cases - w_cost*model$W)
            # d_log_p(v) / d_hid_bias_j
            hid_bias.inc <- momentum*hid_bias.inc + e.h*(sum.row(hid_probs) - sum.row(hid_probs.fantasy))/num.cases
            # d_log_p(v) / d_vis_bias_i
            vis_bias.inc <- momentum*vis_bias.inc + e.v*(sum.row(data) - sum.row(vis_sample.fantasy))/num.cases
            model$W <- model$W + W.inc
            model$hid_bias <- model$hid_bias + hid_bias.inc
            model$vis_bias <- model$vis_bias + vis_bias.inc        
        }            
        #png(filename=sprintf("%d_epoch_%d",epoch,num.vis))        
        #hist(model$W)
        #dev.off()
    }
    
    return(model)
}


