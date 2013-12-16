
max_length <- 300
allm <- NULL
newp=TRUE
par(mfrow=c(1,3))
for(j in 1:3) {
    for(i in 1:10) {
        for(ep in 0:9) {
            m <- read.table(paste("/home/alexeyche/prog/sim/responces/d",j,"_",i,"_ep_",ep,"_resp.csv",sep=""), sep=",")
            
            #plot_rast(m, newp,j)
            #if(newp) { newp=FALSE }
             m<-unlist(m)
            m <- c(m, rep(0, max_length-length(m)))
            allm <- rbind(allm, m)
        }
    }
    #newp=TRUE
}    

    



#t <- kmeans(allm,4)
#plotcluster(allm, t$cluster)
# Determine number of clusters
p <- princomp(allm)
require(rgl)
plot3d(p$scores, col=c(rep(1,100),rep(2,100), rep(4,100)))
