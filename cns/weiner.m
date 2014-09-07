%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%    Parameters for Wiener-Hopf solution
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Lx = 100000;                  % number of samples for simulation
L = 6;                       % number of coefficients in adaptive filter



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%    Setup of system identification architecture
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%x = (randn(Lx,1) + sqrt(-1)*randn(Lx,1))/sqrt(2);
x = sin(2*pi/8*(0:(Lx-1)));
                             % input signal is complex Gaussian noise 
                             %   with unit variance
c = [1; -1];                 % unknown system
d = filter(c,1,x);           % desired signal 



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%    Calculate Wiener-Hopf solution
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
rxx = xcorr(x,L-1)/Lx;      % autocorrelation sequence over (N-1) lags
R   = toeplitz(rxx(L:-1:1),rxx(L:2*L-1));
                            % covariance matrix
rdx = xcorr(d,x,L-1)/Lx;    % cross-correlation sequence over (N-1) lags
p   = (rdx(L:2*L-1))';   % cross-correlation vector
w_opt = inv(R)*p;           % Wiener-Hopf solution 