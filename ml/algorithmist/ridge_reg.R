#!/usr/bin/RScript

source("init_cond.R")

library(MASS)

train <- data.frame(Y, X, X^2, X^3, X^4, X^5)
colnames(train) <- c('Y','X','X2','X3','X4','X5')

stats <- NULL
lambda <- 0
x <- (1:100)/100
test <- as.matrix(cbind(1,x,x^2,x^3,x^4,x^5))

plot(X, Y, ylim=c(-5,15), pch=20, col='red')
points(Xcv, Ycv, pch=20, col='blue')

ls <- NULL
cs <- NULL
for(i in 1:15) {
    ridge <- lm.ridge( Y ~ X+X2+X3+X4+X5, train, lambda=lambda)
    Yp <- as.matrix(cbind(1,X,X^2,X^3,X^4,X^5))  %*% as.matrix(coef(ridge))
    trainError <- sum((Yp-Y)^2)/length(Y)

    Yp <- as.matrix(cbind(1,Xcv,Xcv^2,Xcv^3,Xcv^4,Xcv^5))  %*% as.matrix(coef(ridge))
    cvError <- sum((Yp-Ycv)^2)/length(Ycv)

    stats <- rbind(stats, c(lambda, trainError, cvError))
    if(i%%5==1) {
        y0 <- test %*% as.matrix(coef(ridge))
        lines(x,y0, col=i)
        ls <- c(ls, paste("lambda=",lambda,sep=''))
        cs <- c(cs, i)
    }
    lambda <- ifelse(lambda==0,0.000001, lambda*10)
}

legend("topleft", c("Train", "Cross-validation"), pch=20, col=c('red', 'blue'))  
legend("bottomright",inset=0.05, legend=ls, pch=20, col=cs, text.col=cs, bty="n") 

plot(stats[,2], ylim=c(0,10), type='l', col='red', ylab="error", xlab="log(lambda)")  
lines(stats[,3], col='blue') 
