#!/usr/bin/RScript

source('sys.R')
source('rbm.R')
source('makebatches.R')
source('dbn_util.R')
require(tsne)

set.seed(1)

#data <- read.csv('/home/alexeyche/my/dbn/kaggle_sentiment/training_feat_proc.csv',sep=',',header=FALSE)
data <- read.csv('/home/alexeyche/prog/alexeyche-junk/ml/dbn/sentiment/training_feat_proc.csv',sep=',',header=FALSE)

data <- as.matrix(data)
num.dims.raw <- ncol(data)

data <- data[,2:num.dims.raw]
data.target <- matrix(data[,1], ncol=1)

batch.size <- 100

c(data.b, data.b.t) := makebatches(data = data, target.data = data.target, batch.size = batch.size, normalize=FALSE)

c(num.cases, num.dims, num.batches) := dim(data.b)

train.params = list(e.w = 0.1, e.v = 0.01, e.h = 0.01, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 10, cd.iter = 2, persistent = TRUE)

num.vis <- ncol(data)
num.hid <- 500

model_1 <- train_rbm(data.b, train.params, num.hid)
batched.hid_probs_1 <- collect_hidden_statistics(model_1,data.b)

num.vis <- 500
num.hid <- 2

train.params = list(e.w = 0.001, e.v = 0.001, e.h = 0.001, w_cost = 0.0002, 
                    init.moment = 0.8, fin.moment = 0.8, 
                    epochs = 10, cd.iter = 2, persistent = TRUE)

model_2 <- train_rbm(batched.hid_probs_1, train.params, num.hid, type = 'BG')
batched.hid_probs_2 <- collect_hidden_statistics(model_2, batched.hid_probs_1)


top_hids <- unbatch_data(batched.hid_probs_2)
top_hids.t <- unbatch_data(data.b.t)
colors = rainbow(length(unique(top_hids.t)))
names(colors) =  as.character(unique(top_hids.t))

plot(top_hids,t='n')
text(top_hids,labels=top_hids.t, col=colors[as.character(top_hids.t)])






