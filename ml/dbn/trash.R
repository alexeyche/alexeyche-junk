
contrastive_divergence <- function(data, W, vis_bias, hid_bias, iter = 1) {                                   
    n <- nrow(data)
    # R specific:
    vis_bias_cases <- matrix(rep(vis_bias,n),nrow = n, byrow=TRUE) # imittate bias for each case, just for compute comfort
    hid_bias_cases <- matrix(rep(hid_bias,n),nrow = n, byrow=TRUE)   
    
    vis_states <- data    
    # positive phase
    # p(h|x) calculate hidden states with given visible units state
    hid_prob <- sigmoid( vis_states %*% W + hid_bias_cases) # col for each hidden unit, row for each case
    hid_states <- sample_bernoulli(hid_prob) # col for each hidden unit, row for each case
    for(it in 1:iter) {         
        # negative phase
        # p(v_model|h) calculate visible state with given hidden units state from positive phase
        vis_states.model <- sigmoid( hid_states %*% t(W) + vis_bias_cases )
        # p(h|v_model) calculate hidden state with given visible units
        hid_prob.model <- sigmoid( vis_states %*% W + hid_bias_cases )
        
        vis_states <- vis_states.model
    }
}