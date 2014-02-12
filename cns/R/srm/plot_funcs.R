require(lattice)

plot_rastl <- function(raster, lab="") {
  x <- c()
  y <- c()
  
  for(i in 1:length(raster)) {
    x <- c(x, raster[[i]])
    y <- c(y, rep(i, length(raster[[i]])))
  }
  return(xyplot(y~x,list(x=x, y=y), main=lab, xlim=c(0, max(x))))
} 
require(reshape2)
require(ggplot2)
get_gray_plot <- function(data, lims = c(min(data),max(data))) {
  d = melt(data)
  gg <- ggplot(d,aes(x = d[,1],y = d[,2]))+
    geom_tile(aes(fill=value))+
    scale_fill_gradient(low="black",high="white",limits=lims)+
    coord_equal()
  return(gg)
}

gray_plot <- function(data, lims = c(min(data),max(data)) ) { 
  gg <- get_gray_plot(data, lims)
  plot(gg)
}


plot_run_status = function(net, net_neurons, sim_out, pic_filename, descr) {
    W = get_weights_matrix(net_neurons$l)
    id_n = net_neurons$all_ids
    not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
    
    png(pic_filename, width=1824, height=1024)
    if(!not_fired) 
      p1 = plot_rastl(net, descr)

    p2 = levelplot(W, col.regions=colorRampPalette(c("black", "white")))
    mean_grad = apply(sim_out$stat$dwstat, c(1,2), mean)
    p3 = levelplot(mean_grad, col.regions=colorRampPalette(c("black", "white")))
    #if(!is.null(loss)) {
    #  dfrm = data.frame(x=1:length(loss), y=c(loss))
    #  p4 = xyplot(y~x, data=dfrm, type="l")
    #}
    
    if(!not_fired)
      print(p1, position=c(0, 0.5, 0.5, 1), more=TRUE)
    #if(!is.null(loss))
    #  print(p4, position=c(0, 0, 0.5, 0.25), more=TRUE)
    
    print(p2, position=c(0.5, 0, 1, 0.5), more=TRUE)
    print(p3, position=c(0.5, 0.5, 1, 1))
    dev.off()   
}

