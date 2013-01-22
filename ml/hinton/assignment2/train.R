#!/usr/bin/RScript

source('load_data.R')
source('sys.R')  # := notation

#train <- function(epochs) {
    epochs <- 100
    # SET HYPERPARAMETERS HERE.
    batchsize <- 100  # Mini-batch size.
    learning_rate <- 0.1  # Learning rate default <- 0.1.
    momentum <- 0.9  # Momentum default <- 0.9.
    numhid1 <- 50  # Dimensionality of embedding space default <- 50.
    numhid2 <- 200  # Number of units in hidden layer default <- 200.
    init_wt <- 0.01  # Standard deviation of the normal distribution
                     # which is sampled to get the initial weights default = 0.01
    
    # VARIABLES FOR TRACKING TRAINING PROGRESS.
    show_training_CE_after <- 100
    show_validation_CE_after <- 1000
    c(train_input, train_target, valid_input, valid_target, test_input, test_target, vocab) := load_data(batchsize)
    c(numwords, batchsize, numbatches) := dim(train_input)
    vocab_size <- nrow(vocab)
    
    # INITIALIZE WEIGHTS AND BIASES.
    word_embedding_weights <- init_wt * matrix(rnorm(vocab_size*numhid1), nrow = vocab_size, ncol = numhid1) #randn(vocab_size, numhid1)
    embed_to_hid_weights <- init_wt * matrix( rnorm(numwords * numhid1 * numhid2), nrow = numwords * numhid1, ncol = numhid2) #randn(numwords * numhid1, numhid2)
    hid_to_output_weights <- init_wt * matrix(rnorm(numhid2 * vocab_size), nrow = numhid2, ncol = vocab_size) #randn(numhid2, vocab_size)
    hid_bias <- matrix(0, nrow = numhid2, ncol = 1) #zeros(numhid2, 1)
    output_bias <- matrix(0, nrow = vocab_size, ncol = 1) #zeros(vocab_size, 1)
#}
