#!/usr/bin/RScript

mean_field <- function(data, model, mf_runs = 20) {
    temp_h0 <- sigmoid(data %*% (2*model$W0) + rep.row(model$hid_bias0, model$batch.size))
    temp_h1 <- sigmoid(temp_h0 %*% (model$W1) + rep.row(model$hid_bias1, model$batch.size))
    
    data_mult_W0 <- data %*% model$W0
    for(i in seq(1, mf_runs)) {
        temp_h0_new <- sigmoid(data_mult_W0 +  rep.row(model$hid_bias0, model$batch.size) + temp_h1 %*% t(model$W1))
        temp_h1_new <- sigmoid(temp_h0_new %*% model$W1 + rep.row(model$hid_bias1, model$batch.size))
        
        diff_h0 <- sum(abs(temp_h0_new - temp_h0))/(model$batch.size*model$num.hid0)
        diff_h1 <- sum(abs(temp_h1_new - temp_h1))/(model$batch.size*model$num.hid1)
        
        #cat("Mean field, iteration #",i,", diff_h0:",diff_h0, "diff_h1",diff_h1,"\n")
        if ((diff_h0 < 0.0000001) & (diff_h1 < 0.0000001)) {
            break
        }    
        temp_h0 <- temp_h0_new
        temp_h1 <- temp_h1_new
    }
    return(list(temp_h0, temp_h1))
}

