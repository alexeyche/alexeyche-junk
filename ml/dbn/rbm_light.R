#!/usr/bin/RScript

sigmoid <- function(x) {
    1/(1+exp(-x))
}

sample_bernoulli <- function(mat) {
    dims <- dim(mat)
    matrix(rbinom(prod(dims),size=1,prob=c(mat)),dims[1],dims[2])
}



# p(h_j = 1 | v,w) = sigmoid(sum_j v_j * w_ij - b_j)

# o o o   | j = 3
#  o o    | i = 2
set.seed(0)
num.cases <- 5
num.dims <- 2
err.total <- 0
epsilon.w      <- 0.1   # Learning rate for weights 
epsilon.vb     <- 0.1   # Learning rate for biases of visible units 
epsilon.hb     <- 0.1   # Learning rate for biases of hidden units 
weight_cost  <- 0.0002   
initial_momentum  <- 0.5
final_momentum    <- 0.9


vector <- array(c(1,0,1,1,0,0,0,1,1,1), dim = c(num.cases,num.dims))
v_num <-2
h_num <-3
bias_v <- array(0,dim = c(1,v_num))
bias_h <- array(0,dim = c(1,h_num))
W <- array(0.1*rnorm(v_num*h_num),dim=c(v_num,h_num)) # visible units for row, hidden units for col

# R specific:
bias_h_cases <- matrix(rep(bias_h,num.cases),nrow = num.cases, byrow=TRUE) # imittate bias for each case, just for compute comfort
bias_v_cases <- matrix(rep(bias_v,num.cases),nrow = num.cases, byrow=TRUE)

# positive phase
# calculate hidden states with given visible units state
prob_h.pos <- sigmoid( vector %*% W + bias_h_cases) # col for each hidden unit, row for each case
hid_states.pos <- sample_bernoulli(prob_h.pos) # col for each hidden unit, row for each case

# calculate derivative components from positive phase:
# E[p(h|v)*v], i.e.  <v_i*h_j>_data , for weights gradient
weights.grad.0 <- t(vector) %*% prob_h.pos
# E[p(h|v)] , for hidden bias gradient
bias_h.grad.0 <- apply(prob_h.pos,2,sum)/num.cases
# E[p(v|h)] , for visible bias gradient
bias_v.grad.0 <- apply(vector,2,sum)/num.cases

# negative phase
# calculate visible state with given hidden units state from positive phase
vis_states.neg <- sigmoid( hid_states.pos %*% t(W) + bias_v_cases )
# calculate hidden state with given visible units
prob_h.neg <- sigmoid( vis_states.neg %*% W + bias_h_cases )

# derivative components from negative phase:
# ?????, i.e. <v_i*h_j>_model , for weights gradient
weights.grad.1 <- t(vis_states.neg) %*% prob_h.neg 
# ???? , for hidden bias gradient
bias_h.grad.1 <- apply(prob_h.neg,2,sum)/num.cases
# ???? , for visible bias gradient
bias_v.grad.1 <- apply(vis_states.neg,2,sum)/num.cases

err <- sum((vector - vis_states.neg)^2)
err.total <- err.total + err

# moment stuff 
momentum <- finalmomentum
if (epoch <= 5) {
    momentum <- initialmomentum
}

# update weights and bias
weights.inc <- momentum*weights.inc + epsilon.w * ((weights.grad.0 - weights.grad.1)/num.cases - weight_cost*W)
bias_h.inc  <- momentum*bias_h.inc + epsilon.hb * (bias_h.grad.0 - bias_h.grad.1)
bias_v.inc  <- momentum*bias_v.inc + epsilon.vb * (bias_v.grad.0 - bias_v.grad.1)
W <- weight.inc


# per element explanation
#for(j in 1:h_num) {    
#    Wsum <- 0
#    for(i in 1:v_num) {        
#        Wsum <- Wsum + W[i,j] * vector[i] 
#    }
#    pos_prob_h[j] <- sigmoid(bias_h[j]+Wsum)
#}
