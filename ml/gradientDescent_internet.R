#  ----------------------------------------------------------------------------------
# |PROGRAM NAME: gradient_descent_OLS_R
# |DATE: 11/27/11
# |CREATED BY: MATT BOGARD 
# |PROJECT FILE:              
# |----------------------------------------------------------------------------------
# | PURPOSE: illustration of gradient descent algorithm applied to OLS
# | REFERENCE: adapted from : http://www.cs.colostate.edu/~anderson/cs545/Lectures/week6day2/week6day2.pdf                
# |  and http://www.statalgo.com/2011/10/17/stanford-ml-1-2-gradient-descent/
#  ---------------------------------------------------------------------------------
 
# get data 
rm(list = ls(all = TRUE)) # make sure previous work is clear
ls()
browser()
x0 <- c(1,1,1,1,1) # column of 1's
x1 <- c(1,2,3,4,5) # original x-values
 
# create the x- matrix of explanatory variables
 
x <- as.matrix(cbind(x0,x1))
 
# create the y-matrix of dependent variables
 
y <- as.matrix(c(3,7,5,11,14))
m <- nrow(y)
 
# implement feature scaling
x.scaled <- x
x.scaled[,2] <- (x[,2] - mean(x[,2]))/sd(x[,2])
 
# analytical results with matrix algebra
 solve(t(x)%*%x)%*%t(x)%*%y # w/o feature scaling
 solve(t(x.scaled)%*%x.scaled)%*%t(x.scaled)%*%y # w/ feature scaling
 
# results using canned lm function match results above
summary(lm(y ~ x[, 2])) # w/o feature scaling
summary(lm(y ~ x.scaled[, 2])) # w/feature scaling
 
# define the gradient function dJ/dtheata: 1/m * (h(x)-y))*x where h(x) = x*theta
# in matrix form this is as follows:
grad <- function(x, y, theta) {
  gradient <- (1/m)* (t(x) %*% ((x %*% t(theta)) - y))
  return(t(gradient))
}
 
# define gradient descent update algorithm
grad.descent <- function(x, maxit){
    theta <- matrix(c(0, 0), nrow=1) # Initialize the parameters
 
    alpha = .05 # set learning rate
    for (i in 1:maxit) {
      theta <- theta - alpha  * grad(x, y, theta)   
    }
 return(theta)
}
 
 
# results without feature scaling
print(grad.descent(x,1000))
 
# results with feature scaling
print(grad.descent(x.scaled,1000))
 
# ----------------------------------------------------------------------- 
# cost and convergence intuition
# -----------------------------------------------------------------------
 
# typically we would iterate the algorithm above until the 
# change in the cost function (as a result of the updated b0 and b1 values)
# was extremely small value 'c'. C would be referred to as the set 'convergence'
# criteria. If C is not met after a given # of iterations, you can increase the
# iterations or change the learning rate 'alpha' to speed up convergence
 
# get results from gradient descent
beta <- grad.descent(x,1000)
 
# define the 'hypothesis function'
h <- function(xi,b0,b1) {
 b0 + b1 * xi 
}
 
# define the cost function   
cost <- t(mat.or.vec(1,m))
  for(i in 1:m) {
    cost[i,1] <-  (1 /(2*m)) * (h(x[i,2],beta[1,1],beta[1,2])- y[i,])^2 
  }
 
totalCost <- colSums(cost)
print(totalCost)
 
# save this as Cost1000
cost1000 <- totalCost
 
# change iterations to 1001 and compute cost1001
beta <- (grad.descent(x,1001))
cost <- t(mat.or.vec(1,m))
  for(i in 1:m) {
    cost[i,1] <-  (1 /(2*m)) * (h(x[i,2],beta[1,1],beta[1,2])- y[i,])^2 
  }
cost1001 <- colSums(cost)
 
# does this difference meet your convergence criteria? 
print(cost1000 - cost1001)
