#!/usr/bin/RScript

source('rbm.R')
source('sys.R')
source('makebatches.R')

data <- read.csv('/home/alexeyche/my/dbn/kaggle_sentiment/training_feat.csv',sep=',',header=FALSE)
data <- as.matrix(data)
num.dims.raw <- ncol(data)

data <- data[1:7000,2:num.dims.raw]
data.target <- matrix(data[1:7000,1], ncol=1)

batch.size <- 100

c(data.b, data.b.t) := makebatches(data = data, target.data = data.target, batch.size = batch.size, normalize=FALSE)

c(num.cases, num.dims, num.batches) := dim(data.b)

train.params = list(e.w = 0.02, e.v = 0.05, e.h = 0.05, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 10, cd.iter = 3)   

num.vis <- 2287
num.hid <- 500

c(model_1, batch.pos.hid.probs) := train_rbm(data.b, train.params, num.hid)

num.vis <- 500
num.hid <- 2

data.b <- batch.pos.hid.probs
c(model_2, batch.pos.hid.probs) := train_rbm(data.b, train.params, num.hid)

#for(batch in 1:num.batches) {
#    hidden_top <- prop_up(data.b[,,batch], model)
#    for(i in 1:num.cases) {            
#        color = 'blue'
#        if(data.b.t[i,,batch] == 1) {
#            color = 'red'
#        }        
#        points(hidden_top[i,][1],hidden_top[i,][2], type='p', xlim=c(0,1), ylim=c(0,1), col = color)        
#    }
#}

