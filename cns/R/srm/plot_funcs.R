require(lattice)

plot_rastl <- function(raster, lab="",T0=0, Tmax=Inf) {
  x <- c()
  y <- c()
  
  for(i in 1:length(raster)) {
    rast = raster[[i]]
    rast = rast[rast >= T0]
    rast = rast[rast < Tmax]
    x <- c(x, rast)
    y <- c(y, rep(i, length(rast)))
  }
  return(xyplot(y~x,list(x=x, y=y), main=lab, xlim=c(T0, max(x))))
}

plot_dwt = function(mx) { 
  par(mfrow=c(length(mx@W),1), mar = rep(2,4))
  for(i in length(mx@W):1) {
    plot(mx@W[[i]], type="l")
  }
}

plot_rast_dwt = function(net) {
  par(mfrow=c(length(net),1), mar = rep(2,4))
  for(i in 1:length(net)) {
    x = net[[i]]
    y = rep(i, length(x))
    plot(x,y)
  }
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

Istat = NULL

plot_run_status = function(net, neurons, loss, pic_filename, descr) {
    W = neurons$Wm()
    id_n = neurons$ids()
    not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
    
    png(pic_filename, width=1024, height=768)
    if(!not_fired) 
      p1 = plot_rastl(net, descr)

    p2 = levelplot(W, col.regions=colorRampPalette(c("black", "white")))
#    mean_grad = apply(sim_out$stat$dwstat, c(1,2), mean)
#    p3 = levelplot(mean_grad, col.regions=colorRampPalette(c("black", "white")))
#    if(!is.null(loss)) {
#     dfrm = data.frame(x=1:length(loss), y=c(loss))
#     p4 = xyplot(y~x, data=dfrm, type="l")
#    }
#    Istat <<- cbind(Istat, sim_out$stat$Istat)
#    Idf = data.frame(t(Istat))
    #Idf = melt(Idf, measure.vars = names(Idf),  variable.name = 'grp', value.name = 'y') 
#    z = 1:ncol(Istat)
#    p5 = xyplot(X1+X2+X3+X4+X5+X6+X7+X8+X9+X10~z, data=Idf, type="l")
    if(!not_fired)
      print(p1, position=c(0, 0.33, 0.5, 1), more=TRUE)
#    if(!is.null(loss))
#      print(p4, position=c(0,0.33, 0.5, 0.66), more=TRUE)
#    print(p5, position=c(0,0,0.5,0.33), more=TRUE)
    print(p2, position=c(0.5, 0, 1, 1)) #, more=TRUE)
#    print(p3, position=c(0.5, 0.5, 1, 1))
    dev.off()   
}

#wstat.df = NULL
#for(x in 1:dim(sim_out$stat$wstat)[1]) {
#  for(y in 1:dim(sim_out$stat$wstat)[2]) {
#    wstat.df = cbind(wstat.df, sim_out$stat$wstat[x,y,])
#  }
#}

#wstat.df = as.data.frame(wstat.df)
#wstat.df = melt(wstat.df, measure.vars = names(wstat.df),  variable.name = 'grp', value.name = 'y') 
#z = 1:dim(sim_out$stat$wstat)[3]
#wstat.df$z = z
#xyplot( y ~ z , data = wstat.df, groups = grp, type="l") 


plot_data_rates = function(net, timeline, labels) {
  N = length(net)
  data_rates = matrix(0, N, length(data_ids))
  for(i in 1:N) {
    for(sp in net[[i]]) {
      tm_sp = timeline - sp      
      patt_id = which(tm_sp == min(tm_sp[tm_sp>0]) )
      data_rates[i, patt_id] = data_rates[i, patt_id] + 1
    }
  }
  ul = unique(labels)
  mean_data_rates = matrix(0, N, length(ul))
  for(lab in ul) {
    mean_data_rates[, which(lab == ul)] = rowMeans(data_rates[, which(labels[data_ids] == lab)])
  }
  x = rep(1:N, length(ul))
  y = c(mean_data_rates)
  cols = c(sapply(ul, function(lab) rep(lab, N)))
  plot(x,y, col=cols)
}