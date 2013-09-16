
# start at c(200,30)
predatorprey <- function(lambda=1, epsilon=.001,delta=1, eta=.001){
function(x,y=NULL){
   if (is.null(y)) {
      y <- x[2]; x <- x[1];
   } 
   dx <- (lambda - epsilon*y)*x;
   dy <- (-delta + eta*x)*y;
   return( c(dx, dy))
 }
}

competition <- function(mu=2, lambda=2, Kx=1000, Ky=500) {
function(x,y=NULL){
  if (is.null(y)) {
      y<- x[2]; x <- x[1];
  }
  dx <- mu*(1-(x+y)/Kx)*x;
  dy <- lambda*(1-(x+y)/Ky)*y;
  return( c(dx, dy) );
}
}

newtoncooling <- function(a1,a2) {
function(x,y=NULL){
  if (is.null(y)) {
      y<- x[2]; x <- x[1];
  }
  dx <- a1*(y-x);
  dy <- a2*(x-y);
  return( c(dx, dy) );
}
}

fhn <- function(gamma=2.5, epsilon=1, a=0.3) {
 function(x,y=NULL){
  if (is.null(y)) {
      y<- x[2]; x <- x[1];
  }
#  gamma = 2.5;
#  epsilon=1;
#  a = 0.3;
  dx <- -x*(x-a)*(x-1) - y;
  dy <- epsilon*(x - gamma*y);
  return( c(dx, dy) );
}
}

disease <- function(b=1,mu=1,C=2){
function(x,y=NULL){
  if (is.null(y)) {
      y<- x[2]; x <- x[1];
  }
  dx <- b-C*x*y;
  dy <- C*x*y - mu*y;
  return( c(dx, dy) );
}
}

linear <- function(x0,y0,a,b,c,d,e=0,f=0) {
  foo <- function(x,y=NULL) {
     if (is.null(y)) {
      y<- x[2]; x <- x[1];
     }
     dx <- a*(x-x0) + b*(y-y0) + e;
     dy <- c*(x-x0) + d*(y-y0) + f;
     return( c(dx, dy) );
  }
}

spring1 <- linear(0,0,0,1,-2,-1);
spring2 <- linear(0,0,0,1,-2,-5);
spring3 <- linear(0,0,0,1,-2,.3);
#pp <- predatorprey(.08, .001, .002, .00002);
pp <- predatorprey(1, .001, 1, .001);
comp <- competition(1,2,1000000,10000000);
linsys <- linear(0,0,.8,.5,-.1,1.0)

eulerxy <- function(fun,x0,tstart=0,tend=1) {
  dt <- min(.005, (tend - tstart)/100);
  nsteps <- round( .5+(tend-tstart)/dt);
  xout <- matrix(0,nsteps+1,length(x0));
  tout <- matrix(0, nsteps+1, 1);
  tout[1] = tstart;
  xout[1,] <- x0;
  for (k in 2:(nsteps+1)) {
    x0 <- x0 + dt*fun(x0);
    xout[k,] <- x0;
    tout[k] <- tout[k-1]+dt;
  }
  return(list(x=xout,t=tout));
}


# Jacobian at a point.
jacobianAtXY <- function(fun,x=NULL, y=NULL){
if (is.null(x) | is.null(y)) {
  x0 <- locator(n=1);
  x <- x0$x; 
  y <- x0$y;  
}
  foo <- fun(x,y);
  h <- .000001;
  foox <- fun(x+h,y);
  fooy <- fun(x,y+h);
  A <- (foox[1] - foo[1])/h;
  B <- (fooy[1] - foo[1])/h;
  C <- (foox[2] - foo[2])/h;
  D <- (fooy[2] - foo[2])/h;
  return(matrix( c(A,B,C,D ), 2,2, byrow=T))
}


# runge-kutta
rk <- function(fun,x0,tstart=0,tend=1) {
  dt <- min(.02, (tend - tstart)/100);
  nsteps <- round( .5+(tend-tstart)/dt);
  xout <- matrix(0,nsteps+1,length(x0));
  tout <- matrix(0,nsteps+1,1);
  tout[1] <- tstart;
  xout[1,] <- x0;
  for (k in 2:(nsteps+1)) {
      k1 <- dt*fun(x0);
      k2 <- dt*fun(x0+k1/2);
      k3 <- dt*fun(x0+k2/2);
      k4 <- dt*fun(x0+k3);
      x0 <- x0 + (k1+k4+(k2+k3)*2)/6;
      xout[k,] <- x0;
      tout[k] <- tout[k-1]+dt;
  }
  return( list(x=xout,t=tout) );
} 


showcontours <- function(fun,xlims, ylims, resol=50,add=F, colors=c('red', 'blue')) {
  x <- matrix(seq(xlims[1],xlims[2], length=resol), byrow=F, resol,resol);
  y <- matrix(seq(ylims[1],ylims[2], length=resol),byrow=T, resol, resol);
  npts = resol*resol;
  z <- fun(x,y);
  z1 <- matrix(z[1:npts], resol, resol);
  z2 <- matrix(z[(npts+1):(2*npts)], resol, resol);
  contour(x[,1],y[1,],z1, add=add, col=colors[1]);
  contour(x[,1],y[1,],z2, add=T, col=colors[2]); 
}

nullclines <- function(fun,xlims, ylims, resol=50, add=F,colors=c('red', 'blue')) {
  x <- matrix(seq(xlims[1],xlims[2], length=resol), byrow=F, resol,resol);
  y <- matrix(seq(ylims[1],ylims[2], length=resol),byrow=T, resol, resol);
  npts = resol*resol;
  z <- fun(x,y);
  z1 <- matrix(z[1:npts], resol, resol);
  z2 <- matrix(z[(npts+1):(2*npts)], resol, resol);
  contour(x[,1],y[1,],z1,levels=c(0), add=add, col=colors[1]);
  contour(x[,1],y[1,],z2,levels=c(0), add=T, col=colors[2]); 
}

phasearrows <- function(fun,xlims,ylims,resol=10, col='black', add=F) {
  if (add==F) {
     plot(1,xlim=xlims, ylim=ylims, type='n');
  }
  x <- matrix(seq(xlims[1],xlims[2], length=resol), byrow=T, resol,resol);
  y <- matrix(seq(ylims[1],ylims[2], length=resol),byrow=F, resol, resol);
  npts <- resol*resol;
  xspace <- abs(diff(xlims))/(resol*5);
  yspace <- abs(diff(ylims))/(resol*5);
  x <- x + matrix(runif(npts, -xspace, xspace),resol,resol);
  y <- y + matrix(runif(npts, -yspace, yspace),resol,resol);
  z <- fun(x,y);
  z1 <- matrix(z[1:npts], resol, resol);
  z2 <- matrix(z[(npts+1):(2*npts)], resol, resol);
  maxx <- max(abs(z1));
  maxy <- max(abs(z2));
  dt <- min( abs(diff(xlims))/maxx, abs(diff(ylims))/maxy)/resol;
  lens <- sqrt(z1^2 + z2^2);
  lens2 <- lens/max(lens); 
  arrows(c(x), c(y), c(x+dt*z1/((lens2)+.1)), c(y+dt*z2/((lens2)+.1)),length=.04, col=col);
}

phasetraj <- function(fun,tdur=1,tstart=0,tend=tstart+tdur,color='orange'){
  x0 <- locator(n=1);
  traj <- rk(fun,c(x0$x, x0$y),tstart,tend);
  points(x0$x,x0$y);  
  lines(traj$x[,1], traj$x[,2], col=color);
}

#nullclines(predatorprey,c(-10,100),c(-10,100),40)
#phasearrows(predatorprey,c(-10,100),c(-10,100),20);
#phasetraj(predatorprey,c(-10,100),c(-10,100),40)
#phasetraj(predatorprey,c(-10,100),c(-10,100),80)

nullclines(fhn(),c(-.3,1.2),c(-.2,.5), 40);
phasearrows(fhn(),c(-.3,1.2),c(-.2,.5), 20);
#phasetraj(fhn(),c(-.3,1.2),c(-.2,.5), 40);
#phasetraj(fhn(),c(-.3,1.2),c(-.2,.5), 40);

#nullclines(disease(),c(-.3,3),c(-.3,3), 40);
#phasearrows(disease(),c(-.3,3),c(-.3,3), 20);


#nullclines(predatorprey, c(-10,2000),c(-10,2000), 80);
#phasearrows(predatorprey, c(-10,2000),c(-10,2000), 20);
#phasetraj(predatorprey, c(-10,2000),c(-10,2000), 80);

#nullclines(competition, c(-10,1100),c(-10,1100), 80);
#phasearrows(competition, c(-10,1100),c(-10,1100), 20);

#nullclines(newtoncooling, c(-10,100), c(-10,100), 50);
#phasearrows(newtoncooling, c(-10,100), c(-10,100), 20);
