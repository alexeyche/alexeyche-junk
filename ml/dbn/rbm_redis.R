#!/usr/bin/RScript

require(rredis)
require(grid)
require(ggplot2)
require(reshape)

source('sys.R')
source('dbn_util.R')

vplayout <- function(x, y) viewport(layout.pos.row = x, layout.pos.col = y)

parse_val <- function(val) {
  as.numeric(strsplit(val, " ")[[1]])
}

getVal <- function(key) {
  as.numeric(strsplit(redisGet(key)," ")[[1]])
}


redisConnect()


get_data <- function(it = NULL) {
  l <- list(list())
  
  if(is.null(it)) {
    patt <- "*"
  } else {
    patt <- paste(it, ":*", sep="")
  }
  keys <- redisKeys(patt)
  vals <- redisMGet(keys)
  for (k in names(vals)) {
    c(it, key, line) := strsplit(k,":")[[1]]    
    line <- as.integer(line)
    if(! it %in% names(l)) {
       l[[it]] <- NULL 
    }
    if(! key %in% names(l[[it]]) ) {
        l[[it]][[key]] <- NULL
    }
    l[[it]][[key]] <- rbind(l[[it]][[key]], parse_val(vals[[k]]))     
  }
  l[[""]] <- NULL
  return(l)
}

monit_plots <- function(d) {  
  get_gray_plot <- function(d, title) {
    ggplot(melt(d),aes(X1,X2))+
      geom_tile(aes(fill=value))+
      scale_fill_gradient(low="black",high="white",limits=c(min(d),max(d)))+
      coord_equal() + labs(title = title)
  } 
  pl_w_inc <- get_gray_plot(d$W_inc, "W_inc")
  pl_w <- get_gray_plot(d$W, "W")
  pl_fen <- ggplot(melt(d$free_en),aes(X2,value)) + geom_point() + geom_line() + 
            ylim(min(d$free_en),0) + labs(title = "Free energy")
  pl_h_m <- get_gray_plot(d$hid_m, "hid_m")
  pl_neg_h_m <- get_gray_plot(d$neg_hid_m, "neg_hid_m")
  pl_neg_v_m <- get_gray_plot(d$neg_vis_m, "neg_vis_m")  
  pl_hist_W <- ggplot(melt(d$W), aes(x=value)) + geom_histogram(binwidth=.5, colour="black", fill="white") + labs(title = "W hist")
  #pl_free_en <- ggplot(melt(d$free_en_acc),aes(X2,value)) + geom_point() + geom_line() +
  #              xlim(0,d$max_epoch) + ylim(min(d$free_en_acc),0) + labs(title = "Free energy acc")
  #pl_free_en_valid_acc <- ggplot(melt(d$free_en_valid_acc),aes(X2,value)) + geom_point() + geom_line() +
  #              xlim(0,d$max_epoch) + ylim(min(d$free_en_acc),0) + labs(title = "Free energy valid acc")
  df <- data.frame(valid = t(d$free_en_valid_acc), en = t(d$free_en_acc), delta = t(d$free_en_valid_delta+d$free_en_valid_acc[1]), eps = seq(1, length(d$free_en_valid_acc)))
  stacked <- with(df, data.frame(value = c(en, valid,delta), variable = factor(rep(c("Train","Valid","Delta"), each = NROW(df))), eps = rep(eps, 3)))
  pl_2_ens <- ggplot(stacked,aes(eps, value, colour = variable)) + geom_line() + xlim(0,d$max_epoch) #+ ylim(min(d$free_en_acc),0) + labs(title = "Free energies acc")

  pl_free_en_valid_delta <- ggplot(melt(d$free_en_valid_delta),aes(X2,value)) + geom_point() + geom_line() +
                xlim(0,d$max_epoch) + ylim(min(d$free_en_valid_delta), max(d$free_en_valid_delta)) + labs(title = "Free energy valid delta")
  
  df <- data.frame(valid = t(d$cost_valid_acc), cost = t(d$cost_acc),  delta = t(d$cost_delta), eps = seq(1, length(d$cost_acc)))
  stacked <- with(df, data.frame(value = c(cost,valid,delta), variable = factor(rep(c("Cost","Cost valid","Delta"), each = NROW(df))), eps = rep(eps, 3)))
  pl_2_costs <- ggplot(stacked,aes(eps, value, colour = variable)) + geom_line() + xlim(0,d$max_epoch) #+ ylim(min(d$free_en_acc),0) + labs(title = "Free energies acc")


  #pl_cost_acc <- ggplot(melt(d$cost_acc),aes(X2,value)) + geom_line() + # + geom_point() + geom_line() +
  #              xlim(0,d$max_epoch) + ylim(0, max(d$cost_acc)) + labs(title = "cost acc")



  print(pl_w, vp = vplayout(1,1))
  print(pl_2_costs, vp = vplayout(1, 2))  
  print(pl_2_ens, vp = vplayout(2,1))
  print(pl_neg_v_m, vp = vplayout(2,2))
  print(pl_h_m, vp = vplayout(2,3))

}

last_it <- -1

x11(width=16, height=9)
grid.newpage()  
pushViewport(viewport(layout = grid.layout(2, 3)))  
while(TRUE) {
  last_it_new <- redisGet("last_it")
  Sys.sleep(0.5)
  if (! is.null(last_it_new)) {
      if(last_it_new != last_it) {
        last_it <- last_it_new
        l <- get_data(last_it)
        monit_plots(l[[last_it]])    
      }
  }
 
}
