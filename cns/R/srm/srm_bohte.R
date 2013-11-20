
ts <- 0.7 # ms
tm <- 10 # ms

epsp <-Vectorize(function(t, fj, fi) {
  if(((t-fj)<0)||(t-fi<0)||(fj<0)) { return(0) }
  (exp(-max(fi-fj,0)/ts)/(1-ts/tm))*(exp(-min(t-fi,t-fj)/tm)-exp(-min(t-fi,t-fj)/ts))
}) # t - s = t-fj - t+fi; min(t-fi, t-fj)

test_epsp <- function() {
  t<-seq(10,30, length.out=500)
  e <- c(); fi <- -Inf; fi_spike <- 19
  for(ti in t) {
    if(ti >= fi_spike) { fi <- fi_spike }
    e <- c(e, epsp(ti,18, fi))
  }
  plot(t,e, type="l")
}  

# absolute(-100 mV) refractoriness with exponential restore
u_abs <- -100 # mV
u_r <- -50 # mV
trf <- 0.25 # ms
trs <- 3 # ms
dr <- 1 # ms
nu <- Vectorize(function(s) {
 if(s<0) { return(0)}
 if(s<dr) { return(u_abs) }
 u_abs*exp(-(s+dr)/trf)+u_r*exp(-s/trs)
})

# stochastic threshold g(u):
beta <- 1
alpha <- 1
tr <- -50 # mV
g <- Vectorize(function(u) {
  (beta/alpha)*(log(1+exp(alpha*(tr-u))) -alpha*(tr-u)) 
})

N <- 10
M <- 1
u_rest <- -70 # mV
w_start <- 5
w <- matrix(rep(w_start, M*N), nrow=M, ncol=N)

x <- vector("list", N)
for(i in 1:N) { x[[i]]=c(-Inf)}
x[[3]] <- c(10,15,20)
x[[4]] <- c(11,16,21)
x[[5]] <- c(13,17,22)
x[[6]] <- c(22,22,23)

u_srm <- function(t, x, y) {
  e_syn <- 0
  for(i in 1:length(x)) {
    e_syn <- e_syn + w[1,i]*sum(epsp(t, x[[i]], max(y)))
  }
  u_rest + e_syn + sum(nu(t-y))
}
plot_u_srm <- function() {
    x <- list()
    x[[1]] <- c(10,16)
    x[[2]] <- c(15,20)
    w <- matrix(30, ncol=2, nrow=1)
    y <- c(-Inf)
    uu <- c()
    t <- seq(0,100, by=0.1)
    for(i in t) {
        if(i > 21) {
            y <- c(-Inf, 21)
        }
        uu <- c(uu, u_srm(i,x,y))
    }
    plot(t, uu, type="l", ylim=c(-90,-20), xlab="t (мс)", ylab="u(t) (мВ)")
}
run_srm <- function(x,y, T=50, dt=0.1) {
  y <- c(-Inf)
  uall <- c()
  pall <- c()
  tsim <- seq(0, 50, by=dt)
  for(t in tsim) {
    curu <- u_srm(t, x, y)
    uall <- c(uall, curu)
    curp <- g(curu)
    pall <- c(pall, curp)
    
    #cat("curu: ", curu, ", curp: ", curp, "\n")
    if(dt*curp>runif(1)) {
      #cat("we had a spike!\n")  
      y <- c(y, t)
    }  
  }
  plot(tsim, pall, type="l")
  plot(tsim, uall, type="l", ylim=c(-70,-45))  
  return(y);
}

prob_of_spikes <- function(x,y) {
  y <- c(-Inf, y)
  p <- 1
  for(ti in y[2:length(y)]) {
    p <- p*g(u_srm(ti, x, max(y[y<ti])))
  }
  return(p)
}

test_prob_of_spikes <- function() {
  t <- seq(1,50, by=0.1)
  pp <- c()
  for(ti in t) {
    pp <- c(pp, prob_of_spikes(x, c(ti,ti+5)))
  }
  plot(t, pp, type="l")
}

prob_of_nospikes <- function(x,y,T) {
  y <- c(-Inf, y)  
  pint <- Vectorize(function(t) { g(u_srm(t, x, max(y[y<t]))) })
  p <- 1
  p <- p*exp(-sum(pint(seq(0, y[2],by=0.1)))*0.1)    
  p <- p*exp(-sum(pint(seq(y[2],T,by=0.1)))*0.1)    
  return(p)
}

pxy <- function(x,y,T) {
  prob_of_spikes(x, y)*prob_of_nospikes(x,y,T)
}

test_pxy <- function() {
  ff <- Vectorize(function(spt) { prob_of_spikes(x, c(spt,spt+5))*prob_of_nospikes(x,c(spt,spt+5),50) })
  t <- seq(0,50,by=1)
  plot(t, ff(t),type="l")
}

entropy <- function(p) {
  p*log(p)
}


