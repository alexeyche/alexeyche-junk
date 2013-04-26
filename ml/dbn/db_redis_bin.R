#!/usr/bin/RScript


require(rredis)
require(grid)
require(ggplot2)
require(reshape)
source('sys.R')


read_mat <- function(k) {
    bb <- redisGet(k,raw=TRUE)
    # reading header, 9999:9999 is maximum
    b_head <- bb[1:10]
    at_index <- which(b_head == '40')
    b_head <- b_head[1:(at_index-1)]
    header <- readBin(b_head, "character")
    spl_h <- strsplit(header,":")[[1]]
    n1 <- as.integer(spl_h[1])
    n2 <- as.integer(spl_h[2])
    bb <- tail(bb, n1*n2*4)
    value <- readBin(bb, "numeric", n=n1*n2,size=4,endian="little")
    i_index <- rep(seq(1:n1), n2)
    j_index <- c(sapply(seq(1:n2),function(x) { rep(x, n1) }))
    mat <- sparseMatrix(i = i_index,j = j_index,x = value)
    return(mat)
}

read_and_assign <- function(keys) {
    for(k in keys) {
        mat <- read_mat(k)
        c(it, key) := strsplit(k,":")[[1]]
        assign(key, mat, envir = .GlobalEnv)
    }
}


monit_plots <- function() {  
    get_gray_plot <- function(d, title) {
        ggplot(melt(d),aes(X1,X2))+
            geom_tile(aes(fill=value))+
            scale_fill_gradient(low="black",high="white",limits=c(min(d),max(d)))+
            coord_equal() + labs(title = title)
    } 
    pl_w_inc <- get_gray_plot(as.matrix(W_inc[1:300,]), "W_inc")
    pl_w <- get_gray_plot(as.matrix(W[1:300,]), "W")
    #pl_fen <- ggplot(melt(as.matrix(free_en)),aes(X2,value)) + geom_point() + geom_line() + 
    #    ylim(min(free_en),0) + labs(title = "Free energy")
    pl_hid_m <- get_gray_plot(as.matrix(hid_m), "hid_m")
    pl_neg_hid_m <- get_gray_plot(as.matrix(neg_hid_m), "neg_hid_m")
    pl_neg_vis_s <- get_gray_plot(as.matrix(neg_vis_s[,1:300]), "neg_vis_s")  
    pl_hist_w <- ggplot(melt(W@x), aes(x=value)) + geom_histogram(binwidth=.5, colour="black", fill="white") + labs(title = "W hist")
    #pl_free_en <- ggplot(melt(free_en_acc),aes(X2,value)) + geom_point() + geom_line() +
    #              xlim(0,max_epoch) + ylim(min(free_en_acc),0) + labs(title = "Free energy acc")
    #pl_free_en_valid_acc <- ggplot(melt(free_en_valid_acc),aes(X2,value)) + geom_point() + geom_line() +
    #              xlim(0,max_epoch) + ylim(min(free_en_acc),0) + labs(title = "Free energy valid acc")
    df <- data.frame(valid = t(free_en_valid_acc), en = t(free_en_acc), delta = t(free_en_valid_delta+free_en_valid_acc[1]), eps = seq(1, length(free_en_valid_acc)))
    stacked <- with(df, data.frame(value = c(en, valid,delta), variable = factor(rep(c("Train","Valid","Delta"), each = NROW(df))), eps = rep(eps, 3)))
    pl_2_ens <- ggplot(stacked,aes(eps, value, colour = variable)) + geom_line() + xlim(0,max_epoch) #+ ylim(min(free_en_acc),0) + labs(title = "Free energies acc")
    
    pl_free_en_valid_delta <- ggplot(melt(free_en_valid_delta),aes(X2,value)) + geom_point() + geom_line() +
        xlim(0,max_epoch) + ylim(min(free_en_valid_delta), max(free_en_valid_delta)) + labs(title = "Free energy valid delta")
    
    df <- data.frame(valid = t(cost_valid_acc), cost = t(cost_acc),  delta = t(cost_delta), eps = seq(1, length(cost_acc)))
    stacked <- with(df, data.frame(value = c(cost,valid,delta), variable = factor(rep(c("Cost","Cost valid","Delta"), each = NROW(df))), eps = rep(eps, 3)))
    pl_2_costs <- ggplot(stacked,aes(eps, value, colour = variable)) + geom_line() + xlim(0,max_epoch) #+ ylim(min(free_en_acc),0) + labs(title = "Free energies acc")
    
    
    #pl_cost_acc <- ggplot(melt(d$cost_acc),aes(X2,value)) + geom_line() + # + geom_point() + geom_line() +
    #              xlim(0,d$max_epoch) + ylim(0, max(d$cost_acc)) + labs(title = "cost acc")
    
    
    
    print(pl_w, vp = vplayout(1,1))
    print(pl_2_costs, vp = vplayout(1, 2))  
    print(pl_2_ens, vp = vplayout(2,1))
    print(pl_neg_v_m, vp = vplayout(2,2))
    print(pl_h_m, vp = vplayout(2,3))
    
}

last_it <- -1
#x11(width=16, height=9)
#grid.newpage()  
#pushViewport(viewport(layout = grid.layout(2, 3)))  
iterate <- function(it="0") {    
    patt <- paste(it, ":*", sep="")
    keys <- redisKeys(patt)
    read_and_assign(keys)
}

refresh <- function() {
    redisConnect()    
    it <- redisGet("last_it")
    cat("Refresh iteration: ", it,"\n")
    iterate(it)
}


while(TRUE) {
    last_it_new <- redisGet("last_it")
    Sys.sleep(0.5)
    if (! is.null(last_it_new)) {
        if(last_it_new != last_it) {
            last_it <- last_it_new
            iterate(last_it)
        }
    }    
}

