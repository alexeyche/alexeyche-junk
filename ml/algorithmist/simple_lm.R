#!/usr/bin/RScript

source("init_cond.R")

train <- data.frame(Y, X, X^2, X^3, X^4, X^5)
colnames(train) <- c('Y','X','X2','X3','X4','X5')

simple <- lm(Y ~ X+X2+X3+X4+X5, train)
error <- sum((predict(simple, train) - Y)^2)/length(Y)
cat("Train error:", error,"\n")

cv <- data.frame(Xcv,Xcv^2,Xcv^3,Xcv^4,Xcv^5)
colnames(cv) <- c('X','X2','X3','X4','X5')
error <- sum((predict(simple,cv)-Ycv)^2)/length(Ycv)
cat("Cross validation error:", error,"\n")

x <- (1:100)/100
test = data.frame(x,x^2,x^3,x^4,x^5)
names(test) <- c('X','X2','X3','X4','X5')
y0 <- predict(simple,test)
plot(X,Y,ylim=c(0,40),xlim=c(0,1))
lines(x,y0,col="red")

