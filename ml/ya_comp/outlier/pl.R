#!/usr/bin/RScript
cl <- read.csv("clusts.csv", header=F)
data <- read.csv("test.csv", header=F)

n <- ncol(cl)
m <- nrow(cl)

#par(mfrow=c(d,d))
set.seed(2)
#jpeg('pl.jpg')
cols <- c("red","green","violet","brown","blue","yellow","cyan", "gray",'purple',"maroon","red2","orangered","pink2","ivory3")
cols_big <- colors()
n_cols <- length(cols)
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
    choosed_cols <- cols
    if (length(names(clusts))>length(cols)) {
        print("choosed big")
        choosed_cols <- cols_big
    }        
    for(cl_n in names(clusts)) {
        cl <- clusts[[cl_n]]
        plot_col <- choosed_cols[as.numeric(cl_n)]
        if(cl_n == "0") {
            plot_col <- "black"
        }
        plot(cl[,1],cl[,2],xlim=c(-4,3), ylim=c(-4,3), col=plot_col) 
        par(new=TRUE)
    }


point_point <- function(point) {
    p <- point
    for(i in 1:length(p)) {
        p[i] = p[i] + 1
        points(data[p[i],1],data[p[i],2], col="red", pch=3)
    }
}

plot_p <- F
if(plot_p) {
    point_point(p)
}
#dev.off()
#}    

#plot(d[d[,3] == 1], xlim = c(-1,1), ylim=c(-1,1))
#points(d2[,1],d2[,2], col="red")
#points(d3[,1],d3[,2], col="blue")

