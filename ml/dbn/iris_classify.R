#!/usr/bin/RScript

source('sys.R') # := and stuff
source('makebatches.R')
source('rbm.R')

maxepoch <- 50
num.hid <- 8; numpen <- 500; numpen2 <- 2000

data(iris)
num.classes <- 3

c(batchdata,batchtargets) := makebatches( iris[,1:4], as.matrix(iris[,5]), 50)
c(num.cases, num.dims, num.batches) := dim(batchdata)

cat("Pretraining Layer 1 with RBM:",num.dims,"-",num.hid,"\n")

train.params = list(e.w = 0.1, e.v = 0.1, e.h = 0.1, w_cost = 0.0002, 
                    init.moment = 0.5, fin.moment = 0.9, 
                    epochs = 50, cd.iter = 1)   

c(rbm.model,batch.pos.hid.probs) := train_rbm(batchdata, train.params, num.hid)

w1 <- rbind(rbm.model$W, rbm.model$hid_bias) 
w_class <- 0.1*matrix(rnorm( (ncol(w1)+1)*num.classes ), nrow = (ncol(w1)+1), num.classes)

l1 <- nrow(w1)-1
l2 <- nrow(w_class)-1

test_err <- NULL
train_err <- NULL

maxepoch <- 100

#for(epoch in 1:maxepoch) {
    err <-0
    err_cr <- 0
    counter <- 0
    c(numcases numdims numbatches) := dim(batchdata)
    N <- numcases
#    for(batch in 1:numbatches) {
        batch <-1
        data <- batchdata[,,batch]
        targetdata <- 


#    }
#}
