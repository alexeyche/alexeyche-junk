#!/usr/bin/RScript
require(ggplot2)
require(reshape)

vplayout <- function(x, y) viewport(layout.pos.row = x, layout.pos.col = y)

get_gray_plot <- function(data, lims = c(min(data),max(data))) {
    gg <- ggplot(melt(data),aes(X1,X2))+
        geom_tile(aes(fill=value))+
        scale_fill_gradient(low="black",high="white",limits=lims)+
        coord_equal()
    return(gg)
}
                          
gray_plot <- function(data, lims = c(min(data),max(data)) ) {
    gg <- get_gray_plot(data, lims)
    plot(gg)
}

                          
daydream <- function(model) {
    n.h <- ncol(model$W)    
    n.v <- nrow(model$W)
    test.num <- 10
    vis.states <- matrix(abs(0.001*rnorm(test.num*n.v)),ncol=n.v, nrow=test.num)
    for(i in 1:2000) {            
        vis.states <- gibbs_vhv(vis.states,model)
        if(i %% 100 == 0) {
            gray_plot(vis.states,lim=c(0,1))            
            Sys.sleep(1)            
        }
    }
}

plot_tsne <- function(data,target) {    
    colors = rainbow(length(unique(target)))
    names(colors) =  unique(as.character(target))
    ecb = function(x,y) {         
        plot(x,t='n')
        text(x,labels=target, col=colors[target])
        Sys.sleep(0.1) # for RStudio
    }
    tsne_dbn = tsne(data, epoch_callback = ecb, perplexity=50)  
    return(tsne_dbn)
}

unbatch_data <- function(batched.data, start.batch = 1) {
    whole <- NULL
    c(num.cases, num.dims, num.batches) := dim(batched.data)
    for(batch in start.batch:num.batches) {
        whole <- rbind(whole, matrix(batched.data[,,batch], nrow=num.cases, ncol=num.dims))        
    }    
    return(whole)
}

int_to_bin <- function(x) {
    p <- x
    n <- ceiling(log2(max(x)))
    v <- array(0, dim=c(length(x),n))
    for(i in n:1) {
        p <- p/2    
        rest <- p %% 1
        v[,i] <- rest != 0
        p <- p - rest    
    }
    v[x == 0, ] <- 0
    return(v)
}


