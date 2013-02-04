#!/usr/bin/RScript


# This program trains Restricted Boltzmann Machine in which
# visible, binary, stochastic pixels are connected to
# hidden, binary, stochastic feature detectors using symmetrically
# weighted connections. Learning is done with 1-step Contrastive Divergence.   
# The program assumes that the following variables are set externally:
# maxepoch  -- maximum number of epochs
# numhid    -- number of hidden units 
# batchdata -- the data that is divided into batches (numcases numdims numbatches)
# restart   -- set to 1 if learning starts from beginning 

epsilonw      <- 0.1   # Learning rate for weights 
epsilonvb     <- 0.1   # Learning rate for biases of visible units 
epsilonhb     <- 0.1   # Learning rate for biases of hidden units 
weightcost  <- 0.0002   
initialmomentum  <- 0.5
finalmomentum    <- 0.9

c(numcases, numdims, numbatches) := dim(batchdata)

if (restart == TRUE) {
    restart <- FALSE
    epoch <- 1
    
    # Initializing symmetric weights and biases. 
    vishid     <- 0.1* array(rnorm(numdims * numhid), dim = c(numdims, numhid))
    hidbiases  <- array(0, dim = c(1, numhid))
    visbiases  <- array(0, dim = c(1, numdims))
    
    poshidprobs <- array(0, dim = c(numcases, numhid))
    neghidprobs <- array(0, dim = c(numcases, numhid))
    posprods    <- array(0, dim = c(numdims, numhid))
    negprods    <- array(0, dim = c(numdims, numhid))
    vishidinc   <- array(0, dim = c(numdims, numhid))
    hidbiasinc  <- array(0, dim = c(1,numhid))
    visbiasinc  <- array(0, dim = c(1,numdims))
    batchposhidprobs <- array(0, dim = c(numcases,numhid,numbatches))
}


for (epoch in epoch:maxepoch) {
    printf('epoch %d\r',epoch); 
    errsum <- 0
    for (batch in 1:numbatches) {
        batch <- 1
        printf('epoch %d batch %d\r',epoch,batch); 

######### START POSITIVE PHASE ###################################################
        data <- batchdata[,,batch]
        # p( h_i == 1 | v,W ) = sigma( sum_j v_j * h_ij - b_j)
        poshidprobs <- 1 /(1 + exp(-data %*% vishid - matrix(rep(hidbiases,numcases), nrow = numcases, byrow=TRUE) ))    
        batchposhidprobs[,,batch] <- poshidprobs
        posprods    <- t(data) %*% poshidprobs
        poshidact   <- sum(poshidprobs);
        posvisact <- sum(data);

    }
}
