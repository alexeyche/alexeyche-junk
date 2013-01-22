#!/usr/bin/RScript


library(R.matlab)

load_data <- function(N) { 
    d <- readMat("data.mat")
    d <- d$data[,,]
    
    numdims <- nrow(d$trainData) 
    D <- numdims - 1
    M <- floor(ncol(d$trainData)/N)
    train_input <- array(d$trainData[1:D,1:(N*M)], dim=c(D,N,M))
    train_target <- array(d$trainData[D+1,1:(N*M)], dim=c(1,N,M))
    valid_input <- d$validData[1:D,]
    valid_target <- d$validData[D+1,]
    test_input <- d$testData[1:D,]
    test_target <- d$testData[D+1,]
    vocab <- NULL
    for(i in 1:length(d$vocab)) {
        vocab <- rbind(vocab, d$vocab[[i]])
    }
    list(train_input = train_input, train_target = train_target, valid_input = valid_input, valid_target = valid_target, test_input = test_input, test_target = test_target, vocab = vocab)
}

