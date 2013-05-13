#!/usr/bin/RScript

source('sys.R')
source('makebatches.R')
source('dbn_util.R')

set.seed(2)
num.vis <- 10
num.hid <- 15
num.dims <- num.vis

num.cases <- 500
batch.size <- 50

# gen data
data.all <- NULL
for(c in 1:num.cases) {
    m <- matrix(0, ncol = num.dims)
    p <- sample(seq(0.1,0.9,length.out=10),1)
    c <- sample(3,1)
    for(j in 1:c) {
        rb <- rbinom(10,10,p)
        for(i in 1:length(rb)) {
            m[rb[i]] = m[rb[i]]+1
        }
    }    
    m <- m/max(m)
    data.all <- rbind(data.all, m)
}
c(data.b, data.b.t) := makebatches(data = data.all, target.data = data.all, batch.size = batch.size, normalize=FALSE)
num.batches <- dim(data.b)[3]

prop_up <- function(given_v, model) {    
    sigmoid( given_v %*% model$W + rep.row(model$hid_bias, nrow(given_v)) )
}

prop_down <- function(given_h, model) {
    sigmoid( given_h %*% t(model$W) + rep.row(model$vis_bias, nrow(given_h)) )        
}

model <- list(W = array(0.01*rnorm(num.vis*num.hid,mean=0,sd=1),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
              vis_bias = array(0,dim = c(1,num.vis)), 
              hid_bias = array(0,dim = c(1,num.hid)),
              num.cases = num.cases, batch.size = batch.size)

v <- data.b[1,,1]
#v %*% model$W + rep.row(model$hid_bias, 1)
#v <- c(1,1,1,1,1,1,1,1,1,1)
lim_i <- 4

hid_states <- array(0,dim = c(1,num.hid) )
for(j in 1:num.hid) {    
    for(i in 1:num.vis) {
        delta_i <- abs(i - j)
        k <- 1-delta_i/lim_i          
        cat("k: ",k," ")
        hid_states[j] <- hid_states[j] + v[i] * model$W[i,j] + model$hid_bias[j]
    }
}



