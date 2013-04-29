#!/usr/bin/RScript


source('rbm.R')
source('sys.R')
source('makebatches.R')
source('dbn_util.R')
source('dbm_funcs.R')

set.seed(2)
num.vis <- 10

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

num.hid0 <- 10
num.hid1 <- 5

model <- list(W0 = array(0.01*rnorm(num.vis*num.hid0),dim=c(num.vis,num.hid0)), # visible units for row, hidden units for col
              W1 = array(0.01*rnorm(num.hid0*num.hid1),dim=c(num.hid0,num.hid1)),
              vis_bias = array(0,dim = c(1,num.vis)), 
              hid_bias0 = array(0,dim = c(1,num.hid0)),
              hid_bias1 = array(0,dim = c(1,num.hid1)),
              num.cases = num.cases, batch.size = batch.size, num.hid0 = num.hid0, num.hid1 = num.hid1)


train.params = list(e.w = 0.001, e.v = 0.001, e.h = 0.001, w_cost = 0.0002, mf_runs = 10, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 100, cd.iter = 5)

for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])
# some inits:
W0.inc <- W1.inc <- hid_bias0.inc <- hid_bias1.inc <- vis_bias.inc <- 0

data <- round(array(runif(batch.size*num.vis), dim = c(batch.size,num.vis)))
neg_hid0_probs <- sigmoid(data %*% (2 * model$W0) + rep.row(model$hid_bias0,batch.size))
errsum <-0



for(epoch in 1:epochs) {
    cat("ep #", epoch," e.w:",e.w, " errsum:", errsum, "\n")
    errsum <-0
    for(b in 1:num.batches) {
        data <- data.b[,,b]
       # e.w <- max(e.w/1.000015,0.00010)
      #  e.v <- max(e.w/1.000015,0.00010)
       # e.h <- max(e.w/1.000015,0.00010)
        
        c(pos_hid0, pos_hid1) := mean_field(data, model)
        
        negdata_CD1 <- sigmoid(pos_hid0 %*% t(model$W0) + rep.row(model$vis_bias, batch.size))
        for(i in 1:cd.iter) {
            neg_hid0_states <- sample_bernoulli(neg_hid0_probs)
            
            neg_hid1_probs <- sigmoid(neg_hid0_states %*% model$W1 + rep.row(model$hid_bias1,batch.size))
            neg_hid1_states <- sample_bernoulli(neg_hid1_probs)
            
            neg_data_probs <- sigmoid(neg_hid0_states %*% t(model$W0) + rep.row(model$vis_bias,batch.size))
            neg_data_states <- sample_bernoulli(neg_data_probs)
            
            neg_hid0_probs <- sigmoid(neg_data_states %*% model$W0 + 
                                          neg_hid1_states %*% t(model$W1) + rep.row(model$hid_bias0,batch.size))
        }
        neg_hid1_probs <- sigmoid(neg_hid0_probs %*% model$W1 + rep.row(model$hid_bias1,batch.size))
        err <- sum((data-negdata_CD1)^2)
        errsum <- errsum + err
        momentum <- fin.moment
        if (epoch <= 5) {
            momentum <- init.moment
        }
        
        W0.inc <- momentum*W0.inc + e.w*( ( t(data) %*% pos_hid0 - t(neg_data_states) %*% neg_hid0_probs)/batch.size - w_cost*model$W0)
        W1.inc <- momentum*W1.inc + e.w*( ( t(pos_hid0) %*% pos_hid1 - t(neg_hid0_probs) %*% neg_hid1_probs)/batch.size - w_cost*model$W1)
        hid_bias0.inc <- momentum*hid_bias0.inc + e.h*(sum.row(pos_hid0) - sum.row(neg_hid0_probs))/batch.size
        hid_bias1.inc <- momentum*hid_bias1.inc + e.h*(sum.row(pos_hid1) - sum.row(neg_hid1_probs))/batch.size
        vis_bias.inc <- momentum*vis_bias.inc + e.v*(sum.row(data)-sum.row(neg_data_states))/batch.size
        
        model$W0 <- model$W0 + W0.inc
        model$W1 <- model$W1 + W1.inc
        model$hid_bias0 <- model$hid_bias0 + hid_bias0.inc
        model$hid_bias1 <- model$hid_bias1 + hid_bias1.inc
        model$vis_bias <- model$vis_bias + vis_bias.inc
    }
}








