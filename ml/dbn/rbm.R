#!/usr/bin/RScript

require(reshape2)
require(ggplot2)

gray_plot <- function(data, lims = c(min(data),max(data)) ) {
    gg <- ggplot(melt(data),aes(Var1,Var2))+
        geom_tile(aes(fill=value))+
        scale_fill_gradient(low="black",high="white",limits=lims)+
        coord_equal()
    plot(gg)
}
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

daydream <- function(model) {
    n.h <- ncol(model$W)    
    n.v <- nrow(model$W)
    test.num <- 10
    vis.states <- matrix(abs(0.001*rnorm(test.num*n.v)),ncol=n.v, nrow=test.num)
    for(i in 1:2000) {            
        hid.probs <- prop_up(vis.states, model)        
        hid.states <- sample_bernoulli(hid.probs)           
        vis.probs <- prop_down(hid.states,model)
        vis.states <- sample_bernoulli(vis.probs)
        if(i %% 100 == 0) {
            gray_plot(vis.probs,lim=c(0,1))            
            Sys.sleep(1)            
        }
    }
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
    sigmoid( given_v %*% model$W + rep.row(model$hid_bias, nrow(given_v)) )
}

prop_down <- function(given_h, model) {    
    sigmoid( given_h %*% t(model$W) + rep.row(model$vis_bias, nrow(given_h)) )    
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
        model <- list(W = array(0.01*rnorm(num.vis*num.hid),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
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
    #png(filename=sprintf("0_epoch_%d",epoch,num.vis))        
    #hist(model$W)
    #dev.off()
    W.inc <- hid_bias.inc <- vis_bias.inc <- 0
    for(epoch in 1:epochs) {               
        for(batch in 1:num.batches) {
            # positive part, given data                                
            data <- data.b[,,batch]
            hid_probs <- prop_up(data,model) # v*W + bias_h    
            if(epoch == epochs) {
                batch.pos.hid.probs[,,batch] <- hid_probs
            }
            hid_probs.w <- hid_probs                
            
            for(cdk.step in 1:cd.iter) {
                hid_states <- sample_bernoulli(hid_probs.w)    
                vis_probs.fantasy <- prop_down(hid_states, model) # h*W' + bias_v
                
                vis_sample.fantasy <- sample_bernoulli(vis_probs.fantasy) # may be replaced by probs
                #vis_sample.fantasy <- vis_probs.fantasy
                hid_probs.w <- prop_up(vis_sample.fantasy, model) # v*W + bias_v
            }            
            hid_probs.fantasy <- hid_probs.w
            
            cost <- cross_entropy_cost(data,vis_probs.fantasy)        
            E.free.mean <- sum(free_energy(data, model))/num.cases
            cat("Epoch # ", epoch, "cost: ", cost," free energy: ", E.free.mean, "\n") 
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
    
    return(list(model = model, batch.pos.hid.probs = batch.pos.hid.probs))
}


