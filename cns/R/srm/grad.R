
#source('srm_bohte.R')
N <- 3
M <- 1
w_start <- 10
w <- matrix(rep(w_start, M*N), nrow=M, ncol=N)

y<-c(25)

x <- list()
x[[1]] <- c(18)
x[[2]] <- c(19)
x[[3]] <- c(22)

# stochastic threshold g(u):
beta <- 2
alpha <- 2
tr <- -50 # mV
g <- Vectorize(function(u) {
  (beta/alpha)*(log(1+exp(alpha*(tr-u))) -alpha*(tr-u)) 
})

u_rest <- -70
u_srm <- function(t, x, y) {
  e_syn <- 0
  for(i in 1:length(x)) {
    e_syn <- e_syn + w[1,i]*sum(epsp(t, x[[i]], max(y)))
  }
  u_rest + e_syn + sum(nu(t-y))
}


p_stroke <- function(u) {
  beta/(1+exp(alpha*(tr-u)))
}

dwpos <- c(0,0,0)
for(yval in c(25)) {
  usyn <- u_srm(yval,x,c(-Inf))
  dwposc <- (p_stroke(usyn)*(1-g(usyn))/g(usyn))*epsp(yval,c(18,19,20),c(-Inf))
  cat("+ dwp for ",yval, " = ",dwposc, "\n")
  dwpos <- dwpos + dwposc                                                      
}



int_part <- Vectorize(function(t) {
  yc <-c(-Inf)
  if(t>25) { yc <- c(25)}
  p_stroke(u_srm(t,x,yc))*epsp(t,c(30),yc)
})


integrate(int_part, 20, 30)



