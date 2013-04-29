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

daydream <- function(model) {
    grid.newpage()  
    pushViewport(viewport(layout = grid.layout(3, 1)))
    
    test.num <- 50
    data <- round(array(runif(batch.size*num.vis), dim = c(batch.size,num.vis)))
    hid0_probs <- sigmoid(data %*% (model$W0) + rep.row(model$hid_bias0,batch.size))
    
    for(i in 1:10) {
        hid0_states <- sample_bernoulli(hid0_probs)
        hid1_probs <- sigmoid(hid0_states %*% model$W1 + rep.row(model$hid_bias1,batch.size))
        hid1_states <- sample_bernoulli(hid1_probs)
        
        data_probs <- sigmoid(hid0_states %*% t(model$W0) + rep.row(model$vis_bias,batch.size))
        data_states <- sample_bernoulli(data_probs)
        
        cat("i=",i,"\n")
        hid0_probs <- sigmoid(data_states %*% model$W0 + 
                                  hid1_states %*% t(model$W1) + rep.row(model$hid_bias0,batch.size))
        # plot
        gg0 <- get_gray_plot(hid0_probs)
        gg1 <- get_gray_plot(hid1_probs)
        gg_data <- get_gray_plot(data_probs)
        print(gg_data, vp = vplayout(1,1))
        print(gg0, vp = vplayout(2,1))
        print(gg1, vp = vplayout(3,1))        
        Sys.sleep(1)        
    }
}