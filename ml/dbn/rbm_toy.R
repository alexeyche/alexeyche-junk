#!/usr/bin/RScript


source('rbm.R')
source('sys.R')
source('makebatches.R')
source('dbn_util.R')

set.seed(2)
num.vis <- 10
num.hid <- 7
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

# init model

train.params = list(e.w = 0.1, e.v = 0.1, e.h = 0.1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 1000, cd.iter = 3, persistent = TRUE)  

#model <- train_rbm(data.b, train.params, num.hid)
#test <- matrix(0, ncol = num.vis, nrow=100)
#hid_probs <- prop_up(test, model)

#plot(energy_all(test,hid_probs,model), type = 'l')

model <- list(W = array(0.1*rnorm(num.vis*num.hid,mean=0.5,sd=0.3),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
             vis_bias = array(0,dim = c(1,num.vis)), 
             hid_bias = array(0,dim = c(1,num.hid)),
             num.cases = num.cases, batch.size = batch.size)


for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])
W.inc <- hid_bias.inc <- vis_bias.inc <- 0


plot_epoch <- 100

if(persistent) {
    persist.hid_probs <- matrix(0, nrow = batch.size, ncol = num.hid)
}

sparsetarget <- .2
hidmeans <- matrix(sparsetarget, nrow=1, ncol=num.hid)
#model$hid_bias <- #log(sparsetarget/(1-sparsetarget)) * matrix(1, nrow=1, ncol=num.hid)
#sparsecost <- .001
sparsecost <- 0.1
sparsedamping <- .9


for(epoch in 1:epochs) {               
    for(batch in 1:num.batches) {
        # positive part, given data                                
        data <- data.b[,,batch]
        hid_probs <- prop_up(data,model) # v*W + bias_h            
        
        if(persistent) {
            c(vis_sample.fantasy, vis_probs.fantasy, hid_probs.fantasy) := contrastive_divergence(persist.hid_probs, model, cd.iter)
            persist.hid_probs = hid_probs.fantasy
        } else {    
            c(vis_sample.fantasy, vis_probs.fantasy, hid_probs.fantasy) := contrastive_divergence(hid_probs,model, cd.iter)
        }       
        
        cost <- cross_entropy_cost(data,vis_probs.fantasy)        
        E.free.mean <- sum(free_energy(data, model))/batch.size
        cat("Epoch # ", epoch, "cost: ", cost," free energy: ", E.free.mean, "\n") 
        # moment stuff 
        momentum <- fin.moment
        if (epoch <= 5) {
            momentum <- init.moment
        }
        # deravatives over log p(v)
        # d_log_p(v) / d_W_ij
        hidmeans <- sparsedamping * hidmeans + (1-sparsedamping) * sum.row(hid_probs)/batch.size
        sparsegrads <- sparsecost*(rep.row(hidmeans,batch.size)-sparsetarget)
            
        W.inc <- e.w* (momentum*W.inc + ( ( t(data) %*% hid_probs - t(vis_sample.fantasy) %*% hid_probs.fantasy - t(data) %*% sparsegrads)/batch.size - w_cost*model$W) )
        # d_log_p(v) / d_hid_bias_j
        hid_bias.inc <- e.h*(momentum*hid_bias.inc + (sum.row(hid_probs) - sum.row(hid_probs.fantasy) - sum.row(sparsegrads))/batch.size)
        # d_log_p(v) / d_vis_bias_i
        vis_bias.inc <- e.v*(momentum*vis_bias.inc + (sum.row(data) - sum.row(vis_sample.fantasy))/batch.size)
        model$W <- model$W + W.inc
        model$hid_bias <- model$hid_bias + hid_bias.inc
        model$vis_bias <- model$vis_bias + vis_bias.inc        
    }
    if(epoch %% plot_epoch == 0) {
        hist(hid_probs,xlim=c(0,1))        
        Sys.sleep(0.3)
    }
    
}


