nnull <- function(v) {
  return(v[v != 0])
}
N <- 10
max_length <- 300
allm <- NULL
newp=TRUE
all_list <- list()
for(j in 1:3) {
  js <- as.character(j)
  all_list[[js]] <- list()
  for(i in 1:10) {
    is <- as.character(i)  
    all_list[[js]][[is]] <- vector("list", N)
    t <- 0
    for(ep in 0:9) {
            m <- read.table(paste("/home/alexeyche/prog/sim/responces/d",j,"_",i,"_ep_",ep,"_resp.csv",sep=""), sep=",")
            eps <- as.character(ep)
            all_list[[js]][[is]]
            for(ri in 1:nrow(m)) {
              all_list[[js]][[is]][[ri]] <-c(all_list[[js]][[is]][[ri]], nnull(m[ri,]) + t)
            }              
            t <- t + 750
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
#p <- princomp(allm)
#require(rgl)
#plot3d(p$scores, col=c(rep(1,100),rep(2,100), rep(4,100)))
