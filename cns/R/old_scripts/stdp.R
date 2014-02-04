load()
stdp.df <- data.frame(stdp2)
#stdp.df <- data.frame(stdp1)
plot(stdp.df[order(stdp.df$X1),], type="p")


#ww <- seq(0,50,length.out=10)
#filled.contour(z=Hsurf0, x=ww,y=ww)

#plot(probs[1,], type="l")
#for(i in 2:51) { lines(probs[i,])}