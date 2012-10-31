#!/usr/bin/RScript

load('.RData')

p <- par(mfrow = c(4,3))
for( i in 1:10 ) {
    print(i)
    data <- st[[i]]
    plot(data[,1],data[,3], ylim = c(0,max(data[,3])/2), type ="l", col = "blue" )
    lines(data[,1],data[,2], col = "green")
}
