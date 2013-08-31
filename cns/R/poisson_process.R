

getN = function(x,tt=1) {
  if(sum(x)<tt)
    return(NA)
  if(x[1]>tt)
    return(0)
  else
    return(max(which(cumsum(x)<tt)))
}


#x = c(0.1,0.6,0.5,0.3,1.4)
#getN(x)
simPP = function(ntrials,tt=1,lambda=2) {
  maxN = round(qpois(0.99999,tt*lambda))+10 # large enough so that the chance of needing a larger sample is tiny
  x = matrix(rexp(ntrials*maxN,lambda),ntrials,maxN)
  return(apply(x,1,getN,tt=tt))
}
#plot(simPP(1000, 1, 10))
lambda_sin=function(x) 100*(sin(x*pi)+1)
lambda_const=function(x) 10
function(t) integrate(f=lambda_const,lower=0,upper=t)$value

Tmax <- 1000
s=0; v=seq(0,Tmax,length=1000)
X=numeric(1)
while(X[length(X)]<=Tmax){
  u=runif(1)
  s=s-log(u)
  t=min(v[which(Vectorize(Lambda)(v)>=s)])
  X=c(X,t)
}

plot(lambda(v))