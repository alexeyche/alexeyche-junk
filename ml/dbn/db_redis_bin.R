#!/usr/bin/RScript


require(rredis)
require(grid)
require(ggplot2)
require(reshape)
require(Matrix)
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

vplayout <- function(x, y) viewport(layout.pos.row = x, layout.pos.col = y)

monit_plots <- function(what_plot) {  
    get_gray_plot <- function(d, title) {
        ggplot(melt(d),aes(X1,X2))+
            geom_tile(aes(fill=value))+
            scale_fill_gradient(low="black",high="white",limits=c(min(d),max(d)))+
            coord_equal() + labs(title = title)
    } 
    if(what_plot == "hist") {
       hist_hid_m <- ggplot(melt(hid_m@x), aes(x=value)) + geom_histogram(binwidth=.1, colour="black", fill="white") + labs(title = "hidden") + xlim(0,1.1)
       hist_neg_hid_m <- ggplot(melt(neg_hid_m@x), aes(x=value)) + geom_histogram(binwidth=.1, colour="black", fill="white") + labs(title = "hidden") + xlim(0,1.1)
       hist_w <- ggplot(melt(W@x[1:500]), aes(x=value)) + geom_histogram(binwidth=.1, colour="black", fill="white") + labs(title = "W" )
       hist_w_inc <- ggplot(melt(W_inc@x[1:500]), aes(x=value)) + geom_histogram(binwidth=.1, colour="black", fill="white") + labs(title = "W inc")

       print(hist_hid_m, vp = vplayout(1,1))
       print(hist_neg_hid_m, vp = vplayout(1,2))
       print(hist_w, vp = vplayout(2, 1))  
       print(hist_w_inc, vp = vplayout(2,2))
    }
    if(what_plot == "gray_plot") {
        gr_w <- get_gray_plot(as.matrix(W[1:300,]), "W")
        gr_hid_m <- get_gray_plot(as.matrix(hid_m), "hid_m")
        gr_neg_hid_m <- get_gray_plot(as.matrix(neg_hid_m), "neg_hid_m")
        gr_vis_s <- get_gray_plot(as.matrix(vis_s[,1:100]), "vis_s")  
        gr_neg_vis_s <- get_gray_plot(as.matrix(neg_vis_s[,1:100]), "neg_vis_s")  

        print(gr_hid_m, vp = vplayout(1,1))
        print(gr_neg_hid_m, vp = vplayout(1,2))
        print(gr_vis_s, vp = vplayout(2, 1))  
        print(gr_neg_vis_s, vp = vplayout(2,2))
        print(gr_w, vp = vplayout(2,3))
    }
    if(what_plot == "plot") {
        if(length(free_en_valid_acc@x) == 1) {
          df_en <- data.frame(valid = c(free_en_valid_acc@x,free_en_valid_acc@x), 
                           en = c(free_en_acc@x,free_en_acc@x),                          
                           eps = seq(1, 2))
          df_cost <- data.frame(valid = c(cost_valid_acc@x,cost_valid_acc@x), 
                                cost = c(cost_acc@x,cost_acc@x),  
                                eps = seq(1, 2))
        } else {
          df_en <- data.frame(valid = free_en_valid_acc@x, 
                           en = free_en_acc@x,                          
                           eps = seq(1, length(free_en_valid_acc@x)))  
          df_cost <- data.frame(valid = cost_valid_acc@x, 
                                cost = cost_acc@x,  
                                eps = seq(1, length(cost_acc@x)))
        }
        max_pl <- length(free_en_acc@x) 
        stacked <- with(df_en, data.frame(value = c(en, valid), variable = factor(rep(c("Train","Valid"), each = NROW(df_en))), eps = rep(eps, 1)))
        pl_2_ens <- ggplot(stacked,aes(eps, value, colour = variable)) + geom_line() + xlim(0,max_pl) + ylim(min(free_en_acc),0) + labs(title = "Free energies acc")
        
        stacked <- with(df_cost, data.frame(value = c(cost,valid), variable = factor(rep(c("Cost","Cost valid"), each = NROW(df_cost))), eps = rep(eps, 1)))
        pl_2_costs <- ggplot(stacked,aes(eps, value, colour = variable)) + geom_line() + xlim(0,max_pl) + labs(title = "costs acc")
        
        print(pl_2_ens, vp = vplayout(1,1))
        print(pl_2_costs, vp = vplayout(1, 2))  
    }    
}



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

to_plot <- NULL
args <- commandArgs(trailingOnly = TRUE)
if (length(args) != 0) {
    gr <- args[1]
    if (gr == "gray_plot") {
        to_plot <- gr
    }
    if (gr == "hist") {
        to_plot <- gr
    }
    if (gr == "plot") {
        to_plot <- gr
    }
}
#redisConnect()
#max_epoch <- as.integer(redisGet("max_epoch"))

if(! is.null(to_plot)) {
    last_it <- -1
    x11(width=16, height=9)
    grid.newpage()  
    pushViewport(viewport(layout = grid.layout(2, 3)))  
    while(TRUE) {
        redisConnect()
        last_it_new <- redisGet("last_it")
        Sys.sleep(0.5)
        if (! is.null(last_it_new)) {
            if(last_it_new != last_it) {
                last_it <- last_it_new
                iterate(last_it)
                monit_plots(to_plot)
            }
        }    
    }
} else {
    refresh()
}
