

# getN = function(x,tt=1) {
#   if(sum(x)<tt)
#     return(NA)
#   if(x[1]>tt)
#     return(0)
#   else
#     return(max(which(cumsum(x)<tt)))
# }
# 
# 
# #x = c(0.1,0.6,0.5,0.3,1.4)
# #getN(x)
# simPP = function(ntrials,tt=1,lambda=2) {
#   maxN = round(qpois(0.99999,tt*lambda))+10 # large enough so that the chance of needing a larger sample is tiny
#   x = matrix(rexp(ntrials*maxN,lambda),ntrials,maxN)
#   return(apply(x,1,getN,tt=tt))
# }

draw_pp <- function(ntrials, lambda, t_max) {
  k <- 0; t<-0
  k <- numeric(ntrials)
  while(TRUE){
    u=runif(ntrials)
    t=t-log(u)/lambda
    k_cont <- t <= t_max
    if(all(k_cont == FALSE)) { 
      break
    }
    k[k_cont] <- k[k_cont] + 1
  }
  return(k)
}
