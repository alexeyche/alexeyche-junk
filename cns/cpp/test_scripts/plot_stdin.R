#!/usr/bin/RScript

#source("serv.R")
require(stringr)
load()
#filled.contour(x=w1@x, y=w2@x, as.matrix(wz_de), color = terrain.colors)
wz_whole <- array(0, dim=c(50,50,100))
for(i in ls()) {
  m <- str_match(i, "wz_de([0-9]+)")
  if(!is.na(m[2])) {
    wz_whole[,, (as.integer(m[2])+1)] <- as.matrix(get(m[1]))
  }  
}
d<-dim(wz_whole)

for(i in 1:d[3]) {
  filled.contour(x=w1@x, y=w2@x, wz_whole[,,i], color = terrain.colors)
  Sys.sleep(0.1)
}