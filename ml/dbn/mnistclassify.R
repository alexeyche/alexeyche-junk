#!/usr/bin/RScript

source('sys.R') # := and stuff
source('makebatches.R')
source('rbm.R')

maxepoch <- 50
num.hid <- 500; numpen <- 500; numpen2 <- 2000

c(batchdata,batchtargets,testbatchdata,testbatchtargets) := makebatches()
c(num.cases, num.dims, num.batches) := dim(batchdata)

cat("Pretraining Layer 1 with RBM:",num.dims,"-",num.hid,"\n")

train.params = list(e.w = 0.1, e.v = 0.1, e.h = 0.1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 50, cd.iter = 1)   

c(rbm.model,batch.pos.hid.probs) := train_rbm(batchdata, train.params, num.hid)
