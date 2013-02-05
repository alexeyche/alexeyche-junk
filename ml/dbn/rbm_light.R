#!/usr/bin/RScript

source('sys.R')

sigmoid <- function(x) {
    1/(1+exp(-x))
}

sample_bernoulli <- function(mat) {
    dims <- dim(mat)
    matrix(rbinom(prod(dims),size=1,prob=c(mat)),dims[1],dims[2])
}



contrastive_divergence <- function(data, W, vis_bias, hid_bias, iter = 1) {                                   
    num.cases <- nrow(data)
    # R specific:
    vis_bias_cases <- matrix(rep(vis_bias,num.cases),nrow = num.cases, byrow=TRUE) # imittate bias for each case, just for compute comfort
    hid_bias_cases <- matrix(rep(hid_bias,num.cases),nrow = num.cases, byrow=TRUE)   
    
    vis_states <- data    
    for(it in 1:iter) { 
        # positive phase
        # p(h|x) calculate hidden states with given visible units state
        hid_prob <- sigmoid( vis_states %*% W + hid_bias_cases) # col for each hidden unit, row for each case
        hid_states <- sample_bernoulli(hid_prob) # col for each hidden unit, row for each case
        
        # negative phase
        # p(v_model|h) calculate visible state with given hidden units state from positive phase
        vis_states.model <- sigmoid( hid_states %*% t(W) + vis_bias_cases )
        # p(h|v_model) calculate hidden state with given visible units
        hid_prob.model <- sigmoid( vis_states %*% W + hid_bias_cases )
        
        vis_states <- vis_states.model
    }
}


# p(h_j = 1 | v,w) = sigmoid(sum_j v_j * w_ij - b_j)

# o o o   | j = 3
#  o o    | i = 2
set.seed(0)
num.cases <- 5
num.dims <- 2
err.total <- 0
params = list(e.w = 0.1, e.v = 0.1, e.h = 0.1, w_cost = 0.0002)   
initial_momentum  <- 0.5
final_momentum    <- 0.9


data <- array(c(1,0,1,1,0,0,0,1,1,1), dim = c(num.cases,num.dims))
v_num <-2
h_num <-3
vis_bias <- array(0,dim = c(1,v_num))
hid_bias <- array(0,dim = c(1,h_num))
W <- array(0.1*rnorm(v_num*h_num),dim=c(v_num,h_num)) # visible units for row, hidden units for col
W.inc <- hid_bias.inc <- vis_bias.inc <- 0

for (v in 1:length(params)) assign(names(params)[v], params[[v]])

#par(mfrow = c(2,2), mar = par("mar")/2)
#hist(W[1,]); hist(W[2,])

epochs <- 100
for(epoch in 1:epochs) {
    epoch <- 1
    # R specific:
    vis_bias_cases <- matrix(rep(vis_bias,num.cases),nrow = num.cases, byrow=TRUE) # imittate bias for each case, just for compute comfort
    hid_bias_cases <- matrix(rep(hid_bias,num.cases),nrow = num.cases, byrow=TRUE)    
    # positive phase
    # p(h|x) calculate hidden states with given visible units state
    hid_prob <- sigmoid( data %*% W + hid_bias_cases) # col for each hidden unit, row for each case
    hid_states <- sample_bernoulli(hid_prob) # col for each hidden unit, row for each case
    
    # negative phase
    # p(v_model|h) calculate visible state with given hidden units state from positive phase
    vis_states.model <- sigmoid( hid_states %*% t(W) + vis_bias_cases )
    # p(h|v_model) calculate hidden state with given visible units
    hid_prob.model <- sigmoid( vis_states.model %*% W + hid_bias_cases )
    
    err <- sum((data - vis_states.model)^2)
    err.total <- err.total + err
    cat("Epoch #", epoch," err: ", err," total err: ", err.total,"\n") 
    # moment stuff 
    momentum <- final_momentum
    if (epoch <= 5) {
        momentum <- initial_momentum
    }   
    
    # deravatvies over log p(v)
    # d_log_p(v) / d_W_ij
    W.inc <- momentum*W.inc + e.w*( ( t(data) %*% hid_prob - t(vis_states.model) %*% hid_prob.model)/num.cases - w_cost*W)
    # d_log_p(v) / d_hid_bias_j
    hid_bias.inc <- momentum*hid_bias.inc + e.h*(sum.row(hid_prob) - sum.row(hid_prob.model))/num.cases
    # d_log_p(v) / d_vis_bias_i
    vis_bias.inc <- momentum*vis_bias.inc + e.v*(sum.row(data) - sum.row(vis_states.model))/num.cases
    W <- W + W.inc
    hid_bias <- hid_bias + hid_bias.inc
    vis_bias <- vis_bias + vis_bias.inc    
    
}
#hist(W[1,]); hist(W[2,])
# per element explanation
#for(j in 1:h_num) {    
#    Wsum <- 0
#    for(i in 1:v_num) {        
#        Wsum <- Wsum + W[i,j] * vector[i] 
#    }
#    pos_prob_h[j] <- sigmoid(bias_h[j]+Wsum)
#}
