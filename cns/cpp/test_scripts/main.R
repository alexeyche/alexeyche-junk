load()


require(stringr)
l <- list()
for(i in ls()) {
  m <- str_match(i, "raster([0-9]+)")
  if(!is.na(m[2])) {
    l[[m[2]]] <- i
  }
}
par(mfrow=c(1,2))
sl <- sort(names(l))
rast_bef_last <- get(l[[sl[length(sl)-1] ]])
rast_last <- get(l[[sl[length(sl)] ]])

plot_rast <- function(raster, newplot=TRUE, c=1) {
  x <- c()
  y <- c()
  
  for(i in 1:nrow(raster)) {
    spikes <- which(raster[i,]>0)
    x <- c(x, raster[i,spikes])
    y <- c(y, rep(i, length(spikes)))
  }
 if(newplot) { 
     plot(x,y,col=c)
 } else {
     points(x,y,col=c)
 }     
}  
plot_rastl <- function(raster, newplot=TRUE, c=1) {
  x <- c()
  y <- c()
  
  for(i in 1:length(raster)) {
    x <- c(x, raster[[i]])
    y <- c(y, rep(i, length(raster[[i]])))
  }
  if(newplot) { 
    plot(x,y,col=c)
  } else {
    points(x,y,col=c)
  }     
}  
plot_rast(rast_bef_last)
plot_rast(rast_last)
#library(gplots)
#filled.contour(weights)
