#!/usr/bin/RScript

data <- read.csv("test_set",sep="\t",header=F)
data <- data.frame(data, c('NullClass'), stringsAsFactors=F)

n <- ncol(data)
m <- nrow(data)
D <- m
#data <- cbind(data, c('NullClass'))
# for (i in 1:n) {
# 	for(i in 1:m) {
# 		data[i,j]	
# 	}
# }
OverClickLim <- 4
data[data[,1]>OverClickLim,8] <- c('OverClick')
data.c1 <- data[data[,8] == 'OverClick',]
data.c2 <- data[data[,8] == 'NullClass',]
# Pc1d <- nrow(data.f1)/nrow(data)
# Pc2d <- nrow(data.f2)/nrow(data)

classes <- c("Good","Bad")

feat_f1 <- function(d,c) {
	p <- 1/(1+exp(d-4))
	if(c == "Good") {
		p
	}	else {
		1-p
	}
}

P.c.d <- function(lambda,c,d) {
	exp(lambda*feat_f1(d,c))/( exp(lambda*feat_f1(d,classes[1])) + exp(lambda*feat_f1(d,classes[2])) )
}
P.c.d(0.1,c("Good"),data[,1])