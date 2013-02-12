#!/usr/bin/RScript


require(reshape2)
require(ggplot2)
source('rbm.R')
source('sys.R')
source('makebatches.R')

gray_plot <- function(data) {
    gg <- ggplot(melt(data),aes(Var1,Var2))+
        geom_tile(aes(fill=value))+
        scale_fill_gradient(low="black",high="white",limits=c(min(data),max(data)))+
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

plot_data <- function(data,model) {
    n.h <- ncol(model$W)    
    vis.states <- data
    for(i in 1:2000) {    
        hid.probs <- prop_up(vis.states, model)
        
        hid.states <- sample_bernoulli(hid.probs)           
        vis.probs <- prop_down(hid.states,model)
        vis.states <- sample_bernoulli(vis.probs)
    }
    gray_plot(energy_all(vis.states,hid.probs,model))
}


# o o o
#  \|/
#   o o

# o     | j = 1
# o     | i = 1
num.vis <- 1
num.hid <- 3


num.cases <- 5000
num.dims <- 1
err.total <- 0
batch.size <- 10

train.params = list(e.w = 0.1, e.v = 1, e.h = 1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 50, cd.iter = 1)   

set.seed(2)
model <- list(W = array(0.1*rnorm(num.vis*num.hid),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
              vis_bias = array(0,dim = c(1,num.vis)), 
              hid_bias = array(0,dim = c(1,num.hid)),
              num.cases = num.cases, batch.size = batch.size)


#data <- as.matrix(sample(c(rnorm(num.cases/2, mean=0.2, sd=0.03),
#                           rnorm(num.cases/2, mean=0.7, sd=0.03)), num.cases), ncol=1) # mean = 0, sd = 1
data <- matrix(sample(c(rnorm(10000,mean=0,sd=0.6),rnorm(10000,mean=5,sd=1))), ncol=num.vis, nrow=20000)


data <- data[data < 0]
data[data > 5] <- 0

#gray_plot(data)
plot_data(data,model)

readline("Press <Enter> to continue")

for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])

c(data.b, data.b.t) := makebatches(data = data, target.data = data, batch.size = batch.size, normalize=FALSE)

W.inc <- hid_bias.inc <- vis_bias.inc <- 0
maxepoch <- 5000
plot_epoch <- 1000
epoch <- 1
for(epoch in 1:maxepoch) {
    for(batch in 1:10) {
        batch <- 1
        data <- data.b[,,batch]
        # positive part, given data        
        hid_probs <- prop_up(data,model) # v*W + bias_v
        
        hid_probs.w <- hid_probs
        
        cdk.steps <- 1
        for(cdk.step in 1:cdk.steps) {
            hid_states <- sample_bernoulli(hid_probs.w)    
            vis_probs.fantasy <- prop_down(hid_states, model) # h*W' + bias_v
            
            vis_sample.fantasy <- sample_bernoulli(vis_probs.fantasy) # may be replaced by probs
            #vis_sample.fantasy <- vis_probs.fantasy
            hid_probs.w <- prop_up(vis_sample.fantasy, model) # v*W + bias_v
        }
        hid_probs.fantasy <- hid_probs.w    
        
        err <- sum((data - vis_probs.fantasy)^2) 
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
}


