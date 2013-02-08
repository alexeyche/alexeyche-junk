#!/usr/bin/RScript

source('sys.R')
source('rbm.R')

data(iris)

featureNormalize <- function(x) {
    means <- apply(x,2,mean)
    sds <- apply(x,2,sd)
    x <- t((t(x) - means )/ sds)
    return(x)
}

makebatches <- function( data, target_data, batch.size = ceiling(nrow(data)/10), normalize = TRUE ) {
    stopifnot(nrow(data) == nrow(target_data))
    
    if(normalize) {
        data <- featureNormalize(data)
    }
    
    num.dims <- ncol(data)
    num.dims.target <- ncol(target_data)
    tot.num <- nrow(data)
    num.batches <- ceiling(tot.num/batch.size)
    randomorder <- sample(tot.num)
    
    batch.data <- array(0, dim=c(batch.size, num.dims, num.batches))
    batch.targets <- array(0, dim=c(batch.size, num.dims.target, num.batches))
        
    for(b in 1:num.batches) {
        batch.data[,,b] <- data[ randomorder[(1+(b-1)*batch.size):(b*batch.size)] ]
        batch.targets[,,b] <- target_data[ randomorder[(1+(b-1)*batch.size):(b*batch.size)] ]
    }
    return(list(batch.data,batch.targets))
}


c(batched.data, batched.targets) := makebatches(iris[1:4], as.matrix(iris[5]),50)

##################################
num.hid <- 10
params = list(e.w = 0.01, e.v = 0.1, e.h = 0.1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 100, cd.iter = 5)  

c(num.cases, num.dims, num.batches) := dim(batched.data)
num.vis <- num.dims

for (v in 1:length(params)) assign(names(params)[v], params[[v]])


model <- list(W = array(0.1*rnorm(num.vis*num.hid),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
              vis_bias = array(0,dim = c(1,num.vis)), 
              hid_bias = array(0,dim = c(1,num.hid)))

epoch <- 1
batch.pos.hid.probs <- array(0, dim = c(num.cases, num.hid, num.batches))


W.inc <- hid_bias.inc <- vis_bias.inc <- 0
for(epoch in epoch:epochs) {
    #cat("Epoch # ", epoch)
    err.total <- 0
    for(batch in 1:num.batches) {
        data <- batched.data[,,batch]
        # positive phase
        # p(h|x) calculate hidden states with given visible units state
        hid_prob <- sigmoid( data %*% model$W + rep.row(model$hid_bias,num.cases) ) # col for each hidden unit, row for each case
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


