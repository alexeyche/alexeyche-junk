#!/usr/bin/RScript

cl <- read.csv("output0.csv", header=F)
data <- read.csv("test.csv", header=F)

n <- ncol(cl)
m <- nrow(cl)

#par(mfrow=c(d,d))
set.seed(2)

#for(i in 1:n) {
    j <- 1
    max_cl = max(cl[,j])
    clusts <- list()
    for(i in 1:m) {
        clust <- as.character(cl[i,j])
         
        if(is.null(clusts[[clust]])) {
            clusts[[clust]] <- data[i,]   
        } else {
            clusts[[clust]] <- rbind(clusts[[clust]], data[i,])
        }
        
    }
    for(cl in clusts) {
        plot(cl[,1],cl[,2],xlim=c(-4,3), ylim=c(-4,3), col=sample(colors(), 1)) 
        par(new=TRUE)
    }


#}    

#plot(d[d[,3] == 1], xlim = c(-1,1), ylim=c(-1,1))
#points(d2[,1],d2[,2], col="red")
#points(d3[,1],d3[,2], col="blue")

