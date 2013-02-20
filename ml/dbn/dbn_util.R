#!/usr/bin/RScript


gray_plot <- function(data, lims = c(min(data),max(data)) ) {
    gg <- ggplot(melt(data),aes(Var1,Var2))+
        geom_tile(aes(fill=value))+
        scale_fill_gradient(low="black",high="white",limits=lims)+
        coord_equal()
    plot(gg)
}

daydream <- function(model) {
    n.h <- ncol(model$W)    
    n.v <- nrow(model$W)
    test.num <- 10
    vis.states <- matrix(abs(0.001*rnorm(test.num*n.v)),ncol=n.v, nrow=test.num)
    for(i in 1:2000) {            
        hid.probs <- prop_up(vis.states, model)        
        hid.states <- sample_bernoulli(hid.probs)           
        vis.probs <- prop_down(hid.states,model)
        vis.states <- sample_bernoulli(vis.probs)
        if(i %% 100 == 0) {
            gray_plot(vis.probs,lim=c(0,1))            
            Sys.sleep(1)            
        }
    }
}
