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
                    epochs = 10, cd.iter = 1, persistent = FALSE)

num.vis <- ncol(data)
num.hid <- 2000

model_1 <- train_rbm(data.b, train.params, num.hid)
batched.hid_probs_1 <- collect_hidden_statistics(model_1,data.b)

num.vis <- 2000
num.hid <- 500

train.params = list(e.w = 0.01, e.v = 0.01, e.h = 0.01, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 10, cd.iter = 1, persistent = TRUE)

model_2 <- train_rbm(batched.hid_probs_1, train.params, num.hid)
batched.hid_probs_2 <- collect_hidden_statistics(model_2, batched.hid_probs_1)

num.vis <- 500
num.hid <- 10

train.params = list(e.w = 0.01, e.v = 0.01, e.h = 0.01, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 10, cd.iter = 1, persistent = TRUE) 

model_3 <- train_rbm(batched.hid_probs_2, train.params, num.hid)
batched.hid_probs_3 <- collect_hidden_statistics(model_3, batched.hid_probs_2)


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

unbatch_data <- function(batched.data, start.batch = 1, end.batch = 3) {
    whole <- NULL
    c(num.cases, num.dims, num.batches) := dim(batched.data)
    for(batch in start.batch:end.batch) {
        whole <- rbind(whole, matrix(batched.data[,,batch], nrow=num.cases, ncol=num.dims))        
    }    
    return(whole)
}

top_hids <- unbatch_data(batched.hid_probs_3,0,10)
top_hids.t <- unbatch_data(data.b.t,0,10)

tsne_dbn <- plot_tsne(top_hids, top_hids.t)






