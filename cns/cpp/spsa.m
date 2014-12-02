for k=0:n-1
  ak=a/(k+1+A)^alpha;
  ck=c/(k+1)^gamma;
  delta=2*round(rand(p,1))-1;
  thetaplus=theta+ck*delta;
  thetaminus=theta-ck*delta;
  yplus=loss(thetaplus);
  yminus=loss(thetaminus);
  ghat=(yplus-yminus)./(2*ck*delta);
  theta=theta-ak*ghat;
end
theta

%Notes: 
%If maximum and minimum values on the values of theta can be 
%specified, say thetamax and thetamin, then the following two 
%lines can be added below the theta update line to impose the 
%constraints

%  theta=min(theta,thetamax);
%  theta=max(theta,thetamin);

%The MATLAB feval operation (not used above) is useful in 
%yplus and yminus evaluations to allow for easy change of 
%loss function.

%Algorithm initialization not shown above (see discussion
%in introduction to MATLAB code).
