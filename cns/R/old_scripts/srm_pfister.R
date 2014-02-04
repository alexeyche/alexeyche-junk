
e0 <- 1.3 # mV
ts <- 0.7 # ms
tm <- 10 # ms
epsp <- Vectorize(function(s) {
  if(s<0) { return(0) }
  e0*(exp(-s/tm)-exp(-s/ts))
})
nu0 <- -5 # mV
nu <- Vectorize(function(s) {
  if(s<0) { return(0)}
  nu0*exp(-s/tm)
})
#t<-seq(-5,50, length.out=500)
#plot(t, epsp(t), type="l", ylim=c(-5,1))
#lines(t, nu(t))


N <- 10
M <- 1
u_rest <- -70 # mV
w <- matrix(rep(2, M*N), nrow=M, ncol=N)

x <- vector("list", N)
for(i in 1:N) { x[[i]]=c(-Inf)}
x[[3]] <- c(10,15,20)
x[[4]] <- c(11,16,21)
x[[5]] <- c(13,17,22)
x[[6]] <- c(22, 22, 23)

u_srm <- function(t, x, y) {
  e_syn <- 0
  for(i in 1:length(x)) {
    e_syn <- e_syn + w[1,i]*sum(epsp(t-x[[i]]))
  }
  u_rest + e_syn + sum(nu(t-y))
}

u_tresh <- -50 # mV
du <- 3 # mV
p0 <- 1 # 1/ms
g <- Vectorize(function(u) {
  p0*exp((u-u_tresh)/du)
})
#u_prim <- seq(-70,-45, length.out=500)
#plot(u_prim, g(u_prim), type="l")

p <- function(t, x, y) { g(u_srm(t, x, y))}

y <- c(-Inf)
uall <- c()
pall <- c()
tsim <- seq(0, 200, by=0.1)
for(t in tsim) {
  curu <- u_srm(t, x, y)
  uall <- c(uall, curu)
  curp <- g(curu)
  pall <- c(pall, curp)
  
  #cat("curu: ", curu, ", curp: ", curp, "\n")
  if((curp)>runif(1)) {
    #cat("we had a spike!\n")  
    y <- c(y, t)
  }  
}

plot(tsim, pall, type="l")




