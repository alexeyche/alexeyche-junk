#!/usr/bin/RScript

sigmoid <- function(x) {
    1/(1+exp(-x))
}

calc_hidden_probs <- function(data)

# p(h_j = 1 | v,w) = sigmoid(sum_j v_j * w_ij - b_j)

# o o o   | j = 3
#  o o    | i = 2
set.seed(0)
num.cases <- 3
num.dims <- 2
vector <- array(c(1,0,1,1,0,0), dim = c(num.dims,num.cases))
v_num <-2
h_num <-3
bias_v <- array(0,dim = c(1,v_num))
bias_h <- array(0,dim = c(1,h_num))
W <- array(0.1*rnorm(v_num*h_num),dim=c(v_num,h_num)) # visible units for row, hidden units for col

bias_h_cases <- matrix(rep(bias_h,num.cases),nrow = num.cases, byrow=TRUE) # model bias for each case, just for computing comfort
pos_prob_h <- sigmoid( t(vector) %*% W + bias_h_cases) # col for each hidden unit, row for each case
# per element explanation
#for(j in 1:h_num) {    
#    Wsum <- 0
#    for(i in 1:v_num) {        
#        Wsum <- Wsum + W[i,j] * vector[i] 
#    }
#    pos_prob_h[j] <- sigmoid(bias_h[j]+Wsum)
#}