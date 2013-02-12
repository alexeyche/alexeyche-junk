#!/usr/bin/RScript


#if (restart == TRUE) {
#    restart <- FALSE
#    epoch <- 1
#    
#    # Initializing symmetric weights and biases. 
#    vishid     <- 0.1* array(rnorm(numdims * numhid), dim = c(numdims, numhid))
#    hidbiases  <- array(0, dim = c(1, numhid))
#    visbiases  <- array(0, dim = c(1, numdims))
#    
#    poshidprobs <- array(0, dim = c(numcases, numhid))
#    neghidprobs <- array(0, dim = c(numcases, numhid))
#    posprods    <- array(0, dim = c(numdims, numhid))
#    negprods    <- array(0, dim = c(numdims, numhid))
#    vishidinc   <- array(0, dim = c(numdims, numhid))
#    hidbiasinc  <- array(0, dim = c(1,numhid))
#    visbiasinc  <- array(0, dim = c(1,numdims))
#    batchposhidprobs <- array(0, dim = c(numcases,numhid,numbatches))
#}

sigmoid <- function(x) {
    1/(1+exp(-x))
}

sample_bernoulli <- function(probs) {
    dims <- dim(probs)
    matrix(rbinom(prod(dims),size=1,prob=c(probs)),dims[1],dims[2])
}

free_energy <- function(v,model) {    
    - v %*% t(model$vis_bias) - sum.col(log(1+exp(v %*% model$W + rep.row(model$hid_bias,model$batch.size) )) )
}

prop_up <- function(given_v, model) {    
    sigmoid( given_v %*% model$W + rep.row(model$hid_bias, model$batch.size) )
}

prop_down <- function(given_h, model) {    
    sigmoid( given_h %*% t(model$W) + rep.row(model$vis_bias, model$batch.size) )    
}

gibbs_hvh <- function(hid_probs,model) {    
    hid_states <- sample_bernoulli(hid_probs)
    # p(v_model|h) calculate visible state with given hidden units state from positive phase    
    vis_probs <- prop_down(hid_states, model)
    
    vis_states <- sample_bernoulli(vis_probs)
    # p(h|v_model) calculate hidden state with given state of visible units    
    hid_probs <- prop_up(vis_states, model)
    
    list(vis_states = vis_states, hid_probs = hid_probs)
}
gibbs_vhv <- function(vis_sample,model) {    
    hid.probs <- prop_up(vis_sample, model)
    vis.probs <- prop_down(hid_sample, model)
    list(hid.probs = hid.probs,vis.probs = vis.probs)
}

contrastive_divergence <- function(hid_probs, model, iter = 10) {    
    for(it in 1:iter) {
         # col for each hidden unit, row for each case
        c(vis_states, hid_probs) := gibbs_hvh(hid_probs, model)
    }
    return(list(vis_states.end = vis_states, hid_probs.end = hid_probs))
}

train_rbm <- function(batched.data, params, num.hid = NULL, model = NULL) {
    c(num.cases, num.dims, num.batches) := dim(batched.data)
    num.vis <- num.dims
    
    for (v in 1:length(params)) assign(names(params)[v], params[[v]])
    
    if(is.null(model)) {  # need to create new model
        model <- list(W = array(0.1*rnorm(num.vis*num.hid),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
                      vis_bias = array(0,dim = c(1,num.vis)), 
                      hid_bias = array(0,dim = c(1,num.hid)),
                      num.cases = num.cases)
        
        epoch <- 1
        batch.pos.hid.probs <- array(0, dim = c(num.cases, num.hid, num.batches))
    } else {
        if(is.null(num.hid)) {
            cat("Need specified hidden units, or model\n")
        }
    }
    
    W.inc <- hid_bias.inc <- vis_bias.inc <- 0
    for(epoch in epoch:epochs) {
        err.total <- 0
        for(batch in 1:num.batches) {
            data <- batched.data[,,batch]
            # positive phase
            # p(h|x) calculate hidden states with given visible units state
            hid_probs <- prop_up(data, model) # col for each hidden unit, row for each case
            if (epoch == epochs) { # last epoch, saving hidden probabilites
                batch.pos.hid.probs[,,batch] <- hid_probs
            }
            # negative phase
            c( vis_states.model, hid_prob.model) := contrastive_divergence(hid_probs, model, iter = cd.iter)
            err <- sum((data - vis_states.model)^2)
            err.total <- err.total + err
            E.free.mean <- sum(free_energy(data, model))/num.cases
            cat("Epoch # ", epoch, " batch # ", batch," err: ", err," free energy: ", E.free.mean,"\n") 
            # moment stuff 
            momentum <- fin.moment
            if (epoch <= 5) {
                momentum <- init.moment
            }
            
            # deravatives over log p(v)
            # d_log_p(v) / d_W_ij
            W.inc <- momentum*W.inc + e.w*( ( t(data) %*% hid_probs - t(vis_states.model) %*% hid_prob.model)/num.cases - w_cost*model$W)
            # d_log_p(v) / d_hid_bias_j
            hid_bias.inc <- momentum*hid_bias.inc + e.h*(sum.row(hid_probs) - sum.row(hid_prob.model))/num.cases
            # d_log_p(v) / d_vis_bias_i
            vis_bias.inc <- momentum*vis_bias.inc + e.v*(sum.row(data) - sum.row(vis_states.model))/num.cases
            model$W <- model$W + W.inc
            model$hid_bias <- model$hid_bias + hid_bias.inc
            model$vis_bias <- model$vis_bias + vis_bias.inc    
        }
    }
    return(list(model = model, batch.pos.hid.probs = batch.pos.hid.probs))
}


