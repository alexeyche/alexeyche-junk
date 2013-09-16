library(deSolve)

LotVmod <- function (Time, State, Pars) {
  with(as.list(c(State, Pars)), {
    dx = x*(alpha - beta*y)
    dy = -y*(gamma - delta*x)
    return(list(c(dx, dy)))
  })
}

n <- 100 # number of simulations
param.name <- "gamma" # choose parameter to perturb
param.seq <- seq(0,1,length = 50) # choose range of parameters

Pars <- c(alpha = 1, beta = .001, gamma = 1, delta = .001)
Time <- seq(0, 10, length = n)
State <- c(x = .5, y = .9)

param.index <- which(param.name == names(Pars))
out <- list()
for (i in 1:length(param.seq))
  out[[i]] <- matrix(0, n, length(State))

for (i in 1:length(param.seq)) {
  # set params
  Pars.loop <- Pars
  Pars.loop[param.index] <- param.seq[i]
  # converge
  init <- ode(State, Time, LotVmod, Pars.loop)
  # get converged points
  out[[i]] <- ode(init[n,-1], Time, LotVmod, Pars.loop)[,-1]
}

range.lim <- lapply(out, function(x) apply(x, 2, range))
range.lim <- apply(do.call("rbind", range.lim), 2, range)
plot.variable <- "x" # choose which variable to show
plot(0, 0, pch = "", xlab = param.name, ylab = plot.variable,
     xlim = range(param.seq), ylim = range.lim[,plot.variable])
for (i in 1:length(param.seq)) {
  points(rep(param.seq[i], n), out[[i]][,plot.variable])
}