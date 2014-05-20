library(deSolve)
a11 = -1
a12 = 1
a21 = 1
a22 = -1

b1 = 5/4
b2 = 0


A = matrix(c(a11,a12,a21,a22), nrow=2, ncol=2)

LotVmod <- function (Time, State, Pars) {
  with(as.list(c(State, Pars)), {
    dx1 = x1*a11 + x2*a12 + b1
    dx2 = x1*a21 + x2*a22 + b2
    return(list(c(dx1, dx2)))
  })
}

Pars <- c(a11=a11, a12=a12, a21=a21, a22=a22)
Time <- seq(0, 1000, length = 10000)
State <- c(x1 = .0, x2 = .0)

o = ode(func=LotVmod, y=State, parms=Pars, times = Time, method=c("rk4"))
