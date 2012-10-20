#!/usr/bin/RScript

source('init_cond.R')

#Lasso regression
library(lars)

train <- cbind(X, X^2, X^3, X^4, X^5)
colnames(train) <- c('X','X2','X3','X4','X5')
lasso <- lars(train, Y, type='lasso')
 
cv <- cbind(Xcv, Xcv^2, Xcv^3, Xcv^4, Xcv^5)
colnames(cv) <- c('X','X2','X3','X4','X5')

x <- (1:100)*(max(X)*1.1)/100
test = cbind(x,x^2,x^3,x^4,x^5)
colnames(test) <- c('X','X2','X3','X4','X5')

stats <- NULL
lambda <- 0

plot(X,Y,ylim=c(-5,10), pch=20, col="red")
points(Xcv, Ycv, pch=20, col='blue')
ls <- NULL
cs <- NULL
for(i in 1:15) {
    Yp <- predict(lasso, train, s=lambda, type='fit', mode='lambda')
    trainError <- sum((Yp$fit-Y)^2)/length(Y)
    
    Yp <- predict(lasso, cv, s=lambda, type='fit', mode='lambda')
    cvError <- sum((Yp$fit-Ycv)^2)/length(Ycv)

    stats <- rbind(stats, c(lambda, trainError, cvError))
    if(i%%5==1) {
        y0 <- predict(lasso, test, lambda, type='fit', mode='lambda')
        lines(x,y0$fit, col=i)
        ls <- c(ls, paste("lambda=",lambda,sep=''))
        cs <- c(cs, i)
    }

    lambda <- ifelse(lambda == 0,0.1, lambda*2)
}
legend("topleft", c("Train","Cross-validation"), pch=20, col=c('red','blue'))
legend("bottomright",inset=0.05, legend=ls, pch=20, col=cs, text.col=cs, bty="n")

plot(stats[,2], ylim=c(0,5), type="l", col="red", ylab="error", xlab="log(lambda)")
lines(stats[,3], col='blue')
coef(lasso, s=1, mode='lambda')

