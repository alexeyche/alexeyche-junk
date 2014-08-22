% test_STC.m
% 
% script for doing STA/STC analysis with correlated GWN
%
% updated: Sept 20, 2007.  (J. Pillow)

% Create 2 orthogonal filters --------------------
n = 20; 
filt1 = exp(-((.5:n)-(n+5)/2).^2/(n/3))';
filt2 = exp(-((.5:n)-(n+4)/2).^2/(n/3))';
filt1 = filt1./norm(filt1);
filt2 = filt2-filt1*(filt1'*filt2);
filt2 = filt2./norm(filt2);

% Creates stimulus --------
slen = 100000;  % stimulus length
expfilt = exp(-[0:1:15]/3)';  % expt'l filter for making corr GWN
expfilt = expfilt./norm(expfilt);
Stim = conv2(randn(slen+length(expfilt)-1,1), expfilt, 'valid');

% Compute filter responses -------------
x1 = sameconv(Stim,filt1); 
x2 = sameconv(Stim,filt2);  

% Pass filter outputs through nonlinearity ------------
%r = 10*max(x1,0);   % Half-wave rectified nonlinearity
r = 2*x1.^2 + 1.5*x2.^2;  % Squaring nonlinearity

% Generate Poisson spike response ---------------
RefreshRate = 100;  % Stim refresh rate (Hz)
dtbin = .1;          % binsize for Poisson spike generation
rbig = repmat(r'/RefreshRate*dtbin,1./dtbin,1); % make Poisson spike train
sp = sum(rand(size(rbig))<rbig)';

% Do STA/STC analysis ------------------------------
[sta,stc,rawmu,rawcov] = simpleSTC(Stim,sp,n);  

% You can try setting the ridge parameter to values above 1, 
% and getting smoother (but biased) results -- easiest to see w/ STA
ridgeparam = 0;
covInv = inv(rawcov+ridgeparam*eye(n));
covInvsqrt = sqrtm(covInv);

% Whiten STA  -- look at this if using half-rectifying nonlinearity
wsta = covInv*sta;
plot([wsta./norm(wsta) filt1]);

% Whiten STC -- if using quadratic nonlinearity
wstc = covInvsqrt*stc*covInvsqrt;  % Compute "whitened" STC
[u,s,v] = svd(wstc);  

% Take first two filters, map to correct space
k1 = covInvsqrt*u(:,1); 
k2 = covInvsqrt*u(:,2);
k1 = k1./norm(k1); % Normalize amplitude
k2 = k2./norm(k2);

if k1'*filt1 < 0, k1 = -k1; end % change sign of filters to match,
if k2'*filt2 < 0, k2 = -k2; end %   for ease of viz. inspection

%  Plot results -----------
figure(1);
plot(diag(s), 'o'); % examine eigenvalues
title('eigenvalues');

figure(2);
plot(1:n, [filt1 filt2], 1:n, [k1 k2], '--');
title('filters and filter estimates');

% Use larger stim length to see error go down
err = subspace([filt1 filt2], [k1 k2])*180/pi % degrees
