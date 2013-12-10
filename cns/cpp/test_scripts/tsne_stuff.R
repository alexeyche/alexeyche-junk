
i<-1
j<-1
m <- read.csv(paste("/var/tmp/d",j,"_",i,"_resp.csv",sep=""), sep=",")
m<-unlist(m)

i<-2
j<-1
m2 <- read.csv(paste("/var/tmp/d",j,"_",i,"_resp.csv",sep=""), sep=",")
m2<-unlist(m2)

maxv <- max(length(m2),length(m))




t <- tsne(rbind(m[1:420],m2))