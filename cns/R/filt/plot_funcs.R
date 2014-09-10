#!/usr/bin/env Rscript

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
  #plot(x,y, col="black", lwd=2.5, las=1)
  return(xyplot(y~x,list(x=x, y=y), main=lab, xlim=c(T0, max(x)), col="black", lwd=2.5))
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

gr_pl <- function(data, lims = c(min(data),max(data)) ) { 
  gg <- get_gray_plot(data, lims)
  plot(gg)
}

Istat = NULL
#Wacc = vector("list",N)
lossAcc = NULL
simAcc = NULL
discrAcc = NULL

plot_run_status = function(net, neurons, sim, discr, loss, pic_filename, descr) {
    W = neurons$Wm()
    id_n = neurons$ids()
    not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
    
    png(pic_filename, width=1024, height=768)
    if(!not_fired) 
      p1 = plot_rastl(net, descr, T0=0,Tmax=2000)
    
    for(i in 1:ncol(W)) {        
        Wacc[[i]] <<- cbind(Wacc[[i]], W[,i])
    }
    
    p2 = levelplot(W, col.regions=colorRampPalette(c("black", "white")))
    simAcc <<- cbind(simAcc, sim)
    dfrm = data.frame(x=1:length(colMeans(simAcc)), y=c(colMeans(simAcc)))
    p4 = xyplot(y~x, data=dfrm, type="l", main="Similarity")
    
    discrAcc <<- cbind(discrAcc, discr)
    dfrm = data.frame(x=1:length(colMeans(discrAcc)), y=c(colMeans(discrAcc)))
    p5 = xyplot(y~x, data=dfrm, type="l", main="Discripancy")
    
    if(!is.null(loss)) {
        lossAcc <<- c(lossAcc, loss)
        dfrm = data.frame(x=1:length(lossAcc), y=c(lossAcc))
        p3 = xyplot(y~x, data=dfrm, type="l")
    }
    

#    Istat <<- cbind(Istat, sim_out$stat$Istat)
#    Idf = data.frame(t(Istat))
    #Idf = melt(Idf, measure.vars = names(Idf),  variable.name = 'grp', value.name = 'y') 
#    z = 1:ncol(Istat)
#    p5 = xyplot(X1+X2+X3+X4+X5+X6+X7+X8+X9+X10~z, data=Idf, type="l")
    if(!not_fired)
      print(p1, position=c(0, 0.66, 0.5, 1), more=TRUE)
    print(p2, position=c(0, 0, 0.5, 0.33), more=TRUE)
    
    print(p3, position=c(0.5,0.66, 1, 1), more=TRUE)
    print(p4, position=c(0.5,0.33, 1, 0.66), more=TRUE)
    print(p5, position=c(0.5,0, 1, 0.33))
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
  data_rates = matrix(0, N, length(labels))
  for(i in 1:N) {
    for(sp in net[[i]]) {
      if(length(sp)>0) {
        tm_sp = timeline - sp      
        pos_tm = tm_sp>0
        if(any(pos_tm)) {
            patt_id = which(tm_sp == min(tm_sp[pos_tm]) )
            data_rates[i, patt_id] = data_rates[i, patt_id] + 1.0
        }
      }
    }
  }
  ul = unique(labels)
  mean_data_rates = matrix(0, N, length(ul))
  if(ncol(data_rates)>length(ul)) {
      for(lab in ul) {
          mean_data_rates[, which(lab == ul)] = rowMeans(data_rates[, which(labels == lab)])
      }
  } else {
      mean_data_rates = data_rates
  }
      
  x = rep(1:N, length(ul))
  y = c(mean_data_rates)
  cols = c(sapply(ul, function(lab) rep(lab, N)))
  dfrm <- data.frame( y=c(mean_data_rates),
                      x=1:N, 
                      grp=rep(labels,each=N))
  
  return(xyplot(y~x, group=grp, data=dfrm,, col=c("red","blue")))
}
