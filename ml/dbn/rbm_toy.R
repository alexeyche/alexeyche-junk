#!/usr/bin/RScript


source('rbm.R')
source('sys.R')
source('makebatches.R')
source('dbn_util.R')

set.seed(2)
num.vis <- 50
num.hid <- 10
num.dims <- num.vis



train.params = list(e.w = 0.1, e.v = 0.1, e.h = 0.1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 50, cd.iter = 1)



num.cases <- 1000
batch.size <- 100

# gen data
data.all <- NULL
for(c in 1:num.cases) {
    m <- matrix(0, ncol = num.dims)
    rb <- rbinom(50,50,0.9)
    rb2 <- rbinom(50,50,0.1)
    for(i in 1:length(rb)) {
        m[rb[i]] = m[rb[i]]+1
        m[rb2[i]] = m[rb2[i]]+1          
    }
    m <- m/max(m)
    data.all <- rbind(data.all, m)
}
c(data.b, data.b.t) := makebatches(data = data.all, target.data = data.all, batch.size = batch.size, normalize=FALSE)
num.batches <- dim(data.b)[3]

# init model

train.params = list(e.w = 0.1, e.v = 0.1, e.h = 0.1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 20, cd.iter = 1)  

c(model, batch.pos.hid.probs) := train_rbm(data.b, train.params, num.hid)


#model <- list(W = array(0.1*rnorm(num.vis*num.hid,mean=0.5,sd=0.3),dim=c(num.vis,num.hid)), # visible units for row, hidden units for col
#              vis_bias = array(0,dim = c(1,num.vis)), 
#              hid_bias = array(0,dim = c(1,num.hid)),
#              num.cases = num.cases, batch.size = batch.size)


#for (v in 1:length(train.params)) assign(names(train.params)[v], train.params[[v]])
#W.inc <- hid_bias.inc <- vis_bias.inc <- 0
#maxepoch <- 1000
#plot_epoch <- 1000
# for(epoch in 1:maxepoch) {               
#     for(batch in 1:num.batches) {
#         # positive part, given data                                
#         data <- data.b[,,batch]
#         hid_probs <- prop_up(data,model) # v*W + bias_h    
#         hid_probs.w <- hid_probs        
#         
#         cdk.steps <- 1
#         for(cdk.step in 1:cdk.steps) {
#             hid_states <- sample_bernoulli(hid_probs.w)    
#             vis_probs.fantasy <- prop_down(hid_states, model) # h*W' + bias_v
#             
#             vis_sample.fantasy <- sample_bernoulli(vis_probs.fantasy) # may be replaced by probs
#             #vis_sample.fantasy <- vis_probs.fantasy
#             hid_probs.w <- prop_up(vis_sample.fantasy, model) # v*W + bias_v
#         }
#         hid_probs.fantasy <- hid_probs.w
#         
#         cost <- cross_entropy_cost(data,vis_probs.fantasy)        
#         E.free.mean <- sum(free_energy(data, model))/num.cases
#         cat("Epoch # ", epoch, "cost: ", cost," free energy: ", E.free.mean, "\n") 
#         # moment stuff 
#         momentum <- fin.moment
#         if (epoch <= 5) {
#             momentum <- init.moment
#         }
#         # deravatives over log p(v)
#         # d_log_p(v) / d_W_ij
#         W.inc <- momentum*W.inc + e.w*( ( t(data) %*% hid_probs - t(vis_sample.fantasy) %*% hid_probs.fantasy)/num.cases - w_cost*model$W)
#         # d_log_p(v) / d_hid_bias_j
#         hid_bias.inc <- momentum*hid_bias.inc + e.h*(sum.row(hid_probs) - sum.row(hid_probs.fantasy))/num.cases
#         # d_log_p(v) / d_vis_bias_i
#         vis_bias.inc <- momentum*vis_bias.inc + e.v*(sum.row(data) - sum.row(vis_sample.fantasy))/num.cases
#         model$W <- model$W + W.inc
#         model$hid_bias <- model$hid_bias + hid_bias.inc
#         model$vis_bias <- model$vis_bias + vis_bias.inc        
#     }
#     if(epoch %% plot_epoch == 0) {
#         #plot_data(data,model)
#         #readline("Press <Enter> to continue")
#     }
#     
# }


