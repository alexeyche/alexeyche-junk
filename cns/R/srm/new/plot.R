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