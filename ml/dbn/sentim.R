#!/usr/bin/RScript

source('rbm.R')
source('sys.R')
source('makebatches.R')

require(tsne)

set.seed(1)

data <- read.csv('/home/alexeyche/my/dbn/kaggle_sentiment/training_feat_proc.csv',sep=',',header=FALSE)
data <- as.matrix(data)
num.dims.raw <- ncol(data)

data <- data[,2:num.dims.raw]
data.target <- matrix(data[,1], ncol=1)

batch.size <- 100

c(data.b, data.b.t) := makebatches(data = data, target.data = data.target, batch.size = batch.size, normalize=FALSE)

c(num.cases, num.dims, num.batches) := dim(data.b)

train.params = list(e.w = 0.01, e.v = 0.01, e.h = 0.01, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 20, cd.iter = 1)

num.vis <- ncol(data)
num.hid <- 500

c(model_1, batch.pos.hid.probs_1) := train_rbm(data.b, train.params, num.hid)

num.vis <- 500
num.hid <- 250

train.params = list(e.w = 0.01, e.v = 0.01, e.h = 0.02, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 15, cd.iter = 1)  

c(model_2, batch.pos.hid.probs_2) := train_rbm(batch.pos.hid.probs_1, train.params, num.hid)

num.vis <- 250
num.hid <- 10

train.params = list(e.w = 0.01, e.v = 0.01, e.h = 0.01, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 15, cd.iter = 1)  

c(model_3, batch.pos.hid.probs_3) := train_rbm(batch.pos.hid.probs_2, train.params, num.hid)



plot_tsne <- function(data,target) {    
    colors = rainbow(length(unique(target)))
    names(colors) =  unique(as.character(target))
    ecb = function(x,y) {         
        plot(x,t='n')
        text(x,labels=target, col=colors[target])
        Sys.sleep(0.1) # for RStudio
    }
    tsne_dbn = tsne(data, epoch_callback = ecb, perplexity=50)  
    return(tsne_dbn)
}

unbatch_data <- function(batched.data, maxbatches = 3) {
    whole <- NULL
    c(num.cases, num.dims, num.batches) := dim(batched.data)
    for(batch in 1:maxbatches) {
        whole <- rbind(whole, matrix(batched.data[,,batch], nrow=num.cases, ncol=num.dims))        
    }    
    return(whole)
}

top_hids <- unbatch_data(batch.pos.hid.probs_3,10)
top_hids.t <- unbatch_data(data.b.t,10)

tsne_dbn <- plot_tsne(top_hids, top_hids.t)






