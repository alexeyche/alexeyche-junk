#!/usr/bin/RScript


# This method forward propagates through a neural network.
# Inputs:
#   input_batch: The input data as a matrix of size numwords X batchsize where,
#     numwords is the number of words, batchsize is the number of data points.
#     So, if input_batch(i, j) = k then the ith word in data point j is word
#     index k of the vocabulary.
#
#   word_embedding_weights: Word embedding as a matrix of size
#     vocab_size X numhid1, where vocab_size is the size of the vocabulary
#     numhid1 is the dimensionality of the embedding space.
#
#   embed_to_hid_weights: Weights between the word embedding layer and hidden
#     layer as a matrix of soze numhid1*numwords X numhid2, numhid2 is the
#     number of hidden units.
#
#   hid_to_output_weights: Weights between the hidden layer and output softmax
#               unit as a matrix of size numhid2 X vocab_size
#
#   hid_bias: Bias of the hidden layer as a matrix of size numhid2 X 1.
#
#   output_bias: Bias of the output layer as a matrix of size vocab_size X 1.
#
# Outputs:
#   embedding_layer_state: State of units in the embedding layer as a matrix of
#     size numhid1*numwords X batchsize
#
#   hidden_layer_state: State of units in the hidden layer as a matrix of size
#     numhid2 X batchsize
#
#   output_layer_state: State of units in the output layer as a matrix of size
#     vocab_size X batchsize
#


fprop <- function(input_batch, word_embedding_weights, embed_to_hid_weights,
                  hid_to_output_weights, hid_bias, output_bias) {  
  c(numwords, batchsize) := dim(input_batch)
  c(vocab_size, numhid1) := dim(word_embedding_weights)
  numhid2 = ncol(embed_to_hid_weights)
  
  ## COMPUTE STATE OF WORD EMBEDDING LAYER.
  # Look up the inputs word indices in the word_embedding_weights matrix.
  #embedding_layer_state = reshape(
  #                                word_embedding_weights(reshape(input_batch, 1, []),:)',...
  #numhid1 * numwords, []);  
  embedding_layer_state <- matrix(t(word_embedding_weights[as.vector(input_batch),]), nrow = numhid1 * numwords)
  # COMPUTE STATE OF HIDDEN LAYER.
  # Compute inputs to hidden units.
  
  inputs_to_hidden_units <- t(embed_to_hid_weights) %*% embedding_layer_state + matrix(rep(hid_bias,batchsize),nrow=numhid2)
  
  hidden_layer_state <- matrix(0,numhid2, batchsize)
  
  # Apply logistic activation function.
  hidden_layer_state <- 1 / (1 + exp(-inputs_to_hidden_units))
  
  ## COMPUTE STATE OF OUTPUT LAYER.
  # Compute inputs to softmax.
  inputs_to_softmax <- matrix(0, vocab_size, batchsize)
  inputs_to_softmax <- t(hid_to_output_weights) %*% hidden_layer_state
  
  # Subtract maximum. 
  # Remember that adding or subtracting the same constant from each input to a
  # softmax unit does not affect the outputs. Here we are subtracting maximum to
  # make all inputs <= 0. This prevents overflows when computing their
  # exponents.
  inputs_to_softmax <- inputs_to_softmax - apply(inputs_to_softmax,2,max)
  output_layer_state <- exp(inputs_to_softmax)
  output_layer_state <- output_layer_state / matrix(rep(apply(output_layer_state,2,sum),vocab_size), nrow = vocab_size)
  
  list(embedding_layer_state = embedding_layer_state, hidden_layer_state = hidden_layer_state, output_layer_state = output_layer_state)
}
