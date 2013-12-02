load()

x <- c()
y <- c()
for(i in 1:nrow(raster)) {
    spikes <- which(raster[i,]>0)
    x <- c(x, raster[i,spikes])
    y <- c(y, rep(i, length(spikes)))
}

plot(x,y)