library(deSolve)

LotVmod <- function (Time, State, Pars) {
  with(as.list(c(State, Pars)), {
    dx = x*(alpha - beta*y)
    dy = -y*(gamma - delta*x)
    return(list(c(dx, dy)))
  })
}

# please source() from http://www.macalester.edu/~kaplan/math135/pplane.r
nullclines(predatorprey(alpha, beta, gamma, delta),c(-10,100),c(-10,100),40)
phasearrows(predatorprey(alpha, beta, gamma, delta),c(-10,100),c(-10,100),20);

# modification of phasetraj() in pplane.r
draw.traj <- function(func, Pars, tStart=0, tEnd=1, tCut=10, loc.num=1, color = "red") {
  traj <- list()
  print(paste("Click", loc.num, "initial values"))
  x0 <- locator(loc.num, "p")
  for (i in 1:loc.num) {
    out <- as.data.frame(ode(func=func, y=c(x=x0$x[i], y=x0$y[i]), parms=Pars, times = seq(tStart, tEnd, length = tCut)))
    lines(out$x, out$y, col = color)
    traj[[i]] <- out
  }
  return(traj)
}

alpha = 1; beta = .001; gamma = 1; delta = .001

nullclines(predatorprey(alpha, beta, gamma, delta),c(-10,100),c(-10,100),40)
phasearrows(predatorprey(alpha, beta, gamma, delta),c(-10,100),c(-10,100),20, col = "black")
draw.traj(func=LotVmod, Pars=c(alpha = alpha, beta = beta, gamma = gamma, delta = delta), tEnd=10, tCut=100, loc.num=5)