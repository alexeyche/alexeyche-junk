#!/usr/bin/RScript

source('rbm.R')
source('sys.R')

plot_data <- function(data,model) {
    par(mfrow=c(2,2))
    hist(data)
    
    hid.probs <- prop_up(data, model)
    hid.states <- sample_bernoulli(hid.probs)   
    
    hist(data[hid.states == 1], col='red', xlim=c(min(data), max(data)) )    
    hist(data[hid.states == 0], col='blue',xlim=c(min(data), max(data)) )
}


# o    | j = 1
# o     | i = 1
num.vis <- 1
num.hid <- 1


num.cases <- 100
num.dims <- 1
err.total <- 0
train.params = list(e.w = 0.1, e.v = 1, e.h = 1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 50, cd.iter = 1)   

set.seed(0)
model <- list(W = array(0.1*rnorm(num.vis*num.hid),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
              vis_bias = array(0,dim = c(1,num.vis)), 
              hid_bias = array(0,dim = c(1,num.hid)),
              num.cases = num.cases)


data <- as.matrix(rnorm(num.cases, mean=0.5, sd=1), ncol=1) # mean = 0, sd = 1

plot_data(data,model)
readline("Press <Enter> to continue")

for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])


W.inc <- hid_bias.inc <- vis_bias.inc <- 0
maxepoch <- 1000
plot_epoch <- 1000
for(epoch in 1:maxepoch) {
    # positive part, given data
    hid_probs <- prop_up(data,model) # v*W + bias_v
    hid_probs.w <- hid_probs
    
    cdk.steps <- 10
    for(cdk.step in 1:cdk.steps) {
        hid_states <- sample_bernoulli(hid_probs.w)    
        vis_probs.fantasy <- prop_down(hid_states, model) # h*W' + bias_v
        
        vis_sample.fantasy <- sample_bernoulli(vis_probs.fantasy) # may be replaced by probs
        #vis_sample.fantasy <- vis_probs.fantasy
        hid_probs.w <- prop_up(vis_sample.fantasy, model) # v*W + bias_v
    }
    hid_probs.fantasy <- hid_probs.w    
    
    err <- sum((data - vis_sample.fantasy)^2) 
    err.total <- err.total + err
    E.free.mean <- sum(free_energy(data, model))/num.cases
    cat("Epoch # ", epoch, "err: ", err," free energy: ", E.free.mean, " W:", model$W," W.inc:", W.inc,"\n") 
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
    if(epoch %% plot_epoch == 0) {
        plot_data(data,model)
        readline("Press <Enter> to continue")
    }
    
}


