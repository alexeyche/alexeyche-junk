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

free_energy <- function(v,W,hid_bias,vis_bias) {
    n <- nrow(v)    
    - v %*% t(vis_bias) - sum.col(log(1+exp(v %*% W + rep.row(hid_bias,n) )) )
}

prop_up <- function(given_v, model) {    
    sigmoid( given_v %*% model$W + rep.row(model$hid_bias, model$num.cases) )
}

prop_down <- function(given_h, model) {    
    sigmoid( given_h %*% t(model$W) + rep.row(model$vis_bias, model$num.cases) )    
}

gibbs_hvh <- function(hid_sample,model) {    
    # p(v_model|h) calculate visible state with given hidden units state from positive phase
    vis.probs <- prop_down(hid_sample, model)
    # p(h|v_model) calculate hidden state with given probs of visible units
    hid.probs <- prop_up(vis.probs, model)
    list(hid.probs = hid.probs,vis.probs = vis.probs)
}
gibbs_vhv <- function(vis_sample,model) {    
    hid.probs <- prop_up(vis_sample, model)
    vis.probs <- prop_down(hid_sample, model)
    list(hid.probs = hid.probs,vis.probs = vis.probs)
}

contrastive_divergence <- function(hid_prob, model, iter = 10) {    
    for(it in 1:iter) {
        hid_states <- sample_bernoulli(hid_prob) # col for each hidden unit, row for each case
        c(hid_prob, vis_prob) := gibbs_hvh(hid_states,model)
    }
    return(list(hid.probs.end = hid_prob, vis.probs.end = vis_prob))
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
            hid_prob <- prop_up(data, model) # col for each hidden unit, row for each case
            if (epoch == epochs) { # last epoch, saving hidden probabilites
                batch.pos.hid.probs[,,batch] <- hid_prob
            }
            # negative phase
            c(hid_prob.model, vis_prob.model) := contrastive_divergence(hid_prob, model, iter = cd.iter)
            err <- sum((data - vis_prob.model)^2)
            err.total <- err.total + err
            E.free.mean <- sum(free_energy(data, model$W, model$hid_bias, model$vis_bias))/num.cases
            cat("Epoch # ", epoch, " batch # ", batch," err: ", err," free energy: ", E.free.mean,"\n") 
            # moment stuff 
            momentum <- fin.moment
            if (epoch <= 5) {
                momentum <- init.moment
            }
            
            # deravatives over log p(v)
            # d_log_p(v) / d_W_ij
            W.inc <- momentum*W.inc + e.w*( ( t(data) %*% hid_prob - t(vis_prob.model) %*% hid_prob.model)/num.cases - w_cost*model$W)
            # d_log_p(v) / d_hid_bias_j
            hid_bias.inc <- momentum*hid_bias.inc + e.h*(sum.row(hid_prob) - sum.row(hid_prob.model))/num.cases
            # d_log_p(v) / d_vis_bias_i
            vis_bias.inc <- momentum*vis_bias.inc + e.v*(sum.row(data) - sum.row(vis_prob.model))/num.cases
            model$W <- model$W + W.inc
            model$hid_bias <- model$hid_bias + hid_bias.inc
            model$vis_bias <- model$vis_bias + vis_bias.inc    
        }
    }
    return(list(model = model, batch.pos.hid.probs = batch.pos.hid.probs))
}


