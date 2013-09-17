% Convolves (on the fly) the peak times with Gaussians to obtain the instataneous rates, then generates
% spikes from the rates using an inhomogeneous Poisson process
function [sl,r_] = peak2spike(peakList,returnR,rCoef)

global PARAM


% N is the number of "local peaks" taken into account when convolving.
% Farther peaks are supposed to be negligible
if PARAM.speak0 <= 5e-3
    N=7;
elseif PARAM.speak0 <= 10e-3
    N = 11;
elseif PARAM.speak0 <= 20e-3
    N = 13;
elseif PARAM.speak0 <= 25e-3
    N = 16;
else
    N = 19;
end

% deltaT = 3*PARAM.speak0;
nAfferent = double(max(peakList(:,1)));
activePeak = -Inf*ones(nAfferent,N,3);
ip = ones(1,nAfferent);
is=1;
is_ = 1;
sl = zeros(round(1.1*nAfferent*(PARAM.meanFreq*PARAM.rpeak0+PARAM.r0)*PARAM.T),2);

if returnR
    r_ = zeros(floor(PARAM.T/PARAM.dt),nAfferent);
else
    r_ = [];
end

for it=1:floor(PARAM.T/PARAM.dt)
    t = it * PARAM.dt;
    % only "local peaks" are considered in the convolution
    while is<=size(peakList,1) && peakList(is,2) < t+3*(PARAM.speak0+2*PARAM.speak1)
        ip_ = mod(ip(peakList(is,1))-1,N)+1;
        if t-activePeak(peakList(is,1),ip_,1) < 3*activePeak(peakList(is,1),ip_,2)
            warning('Overriding a non-negligible peak. Increase N.')
        end
        activePeak(peakList(is,1), ip_, 1:3 ) = peakList(is,2:4);
        ip(peakList(is,1)) = ip(peakList(is,1))+1;
        is = is+1;
    end

    for n=1:nAfferent
        r = PARAM.r0;
        for p=1:N
            if activePeak(n,p,1)~=-Inf
                 r = r+ activePeak(n,p,3) / activePeak(n,p,2) * P((t-activePeak(n,p,1))/activePeak(n,p,2));
            end
%             r = r+ PARAM.rpeak0 / PARAM.speak0 * P((t-activePeak(n,p))/PARAM.speak0);
        end

        r = r*rCoef(it);
        %save values (for future plotting)
        if returnR
            r_(it,n) = r;
        end
        
        if rand < r * PARAM.dt
            sl(is_,:) = [n t];
            is_ = is_+1;
            if mod(is_,10^5)==0
                timedLog(['t = ' num2str(t) ' s' ])
            end
        end
    end
end

if size(sl,1)>10^3 && is_>size(sl,1)
    warning('Increase sl array size at initialization for better performance')
end
if size(sl,1)>10^3 && is_<.5*size(sl,1)
    warning('Decrease sl array size at initialization for better performance')
end
% remove non used values
sl(is_:end,:)=[];

disp(['Mean frequency = ' num2str(size(sl,1)/nAfferent/PARAM.T) ' Hz' ] )



function P_=P(t)
P_ = (2*pi)^-.5 * exp( - t.^2/2 ); % Gaussian
