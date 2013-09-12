

x<-c(1,2)
y<-c(1,2)
f <- function(x,y) {
  x^2 + y^2
}

#bil_interp <- function(x_c,y_c) {
  x_c <- seq(1,2,length.out=100)
  y_c <- 1.5
  f_r1 <- f(x[1],y[1])*(x[2]-x_c)/(x[2]-x[1]) + f(x[2],y[1])*(x_c-x[1])/(x[2]-x[1])
  f_r2 <- f(x[1],y[2])*(x[2]-x_c)/(x[2]-x[1]) + f(x[2],y[2])*(x_c-x[1])/(x[2]-x[1])
  f_r1*(y[2]-y_c)/(y[2]-y[1]) + f_r2*(y_c-y[1])/(y[2]-y[1])
#}