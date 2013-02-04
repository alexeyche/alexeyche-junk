#!/usr/bin/RScript

source('sys.R') # := and stuff
source('makebatches.R')

maxepoch <- 50
numhid <- 500; numpen <- 500; numpen2 <- 2000

c(batchdata,batchtargets,testbatchdata,testbatchtargets) := makebatches()
c(numcases, numdims, numbatches) := dim(batchdata)

printf('Pretraining Layer 1 with RBM: %d-%d \n',numdims,numhid)
restart=1
