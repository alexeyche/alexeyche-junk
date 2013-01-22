#!/usr/bin/RScript

source('sys.R')  # := notation
source('load_data.R')
source('fprop.R')

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
    
    # LOAD DATA
    c(train_input, train_target, valid_input, valid_target, test_input, test_target, vocab) := load_data(batchsize)
    c(numwords, batchsize, numbatches) := dim(train_input)
    vocab_size <- nrow(vocab)
    
    # INITIALIZE WEIGHTS AND BIASES.
    word_embedding_weights <- init_wt * matrix(rnorm(vocab_size*numhid1), vocab_size, numhid1) #randn(vocab_size, numhid1)
    embed_to_hid_weights <- init_wt * matrix( rnorm(numwords * numhid1 * numhid2), numwords * numhid1, numhid2) #randn(numwords * numhid1, numhid2)
    hid_to_output_weights <- init_wt * matrix(rnorm(numhid2 * vocab_size), numhid2, vocab_size) #randn(numhid2, vocab_size)
    hid_bias <- matrix(0, numhid2, 1) #zeros(numhid2, 1)
    output_bias <- matrix(0, vocab_size, 1) #zeros(vocab_size, 1)

    word_embedding_weights_delta <- matrix(0, vocab_size, numhid1)
    word_embedding_weights_gradient <- matrix(0, vocab_size, numhid1)
    embed_to_hid_weights_delta <- matrix(0, numwords * numhid1, numhid2)
    hid_to_output_weights_delta <- matrix(0, numhid2, vocab_size)
    hid_bias_delta <- matrix(0, numhid2, 1)
    output_bias_delta <- matrix(0, vocab_size, 1)
    expansion_matrix <- diag(1, vocab_size)
    count <- 0
    tiny <- exp(-30)
    
    for(epoch in 1:epochs) {
        cat("Epoch num: ", epoch, "\n")
        this_chunk_CE <- 0
        trainset_CE <- 0
        # LOOP OVER MINI-BATCHES.
        for(m in 1:numbatches) {
            input_batch <- train_input[,,m]
            target_batch <- train_target[,,m]
       
            # FORWARD PROPAGATE.
            # Compute the state of each layer in the network given the input batch
            # and all weights and biases
            c(embedding_layer_state, hidden_layer_state, output_layer_state) := fprop(input_batch, word_embedding_weights, embed_to_hid_weights, 
                                                                                      hid_to_output_weights, hid_bias, output_bias);
            break 
        }
        break
    }

#}
