% spikeList = [ afferent_1, time_1 ;
%               afferent_2, time_2 ;
%               ... ]
% peakList = [ afferent_1, time_1, sigma_1, rho_1 ;
%              afferent_2, time_2, sigma_2, rho_2 ;
%               ... ]

clear all

param

% generate list of peaks
rand('state',PARAM.randState)
randn('state',PARAM.randState)
generatePeak

% % tmp
% for i=1:size(peakList,1)
%     if peakList(i,1)<=round(PARAM.nAfferentInPattern/2)
%         peakList(i,3)=5e-3;
%     elseif peakList(i,1)<=PARAM.nAfferentInPattern
%         peakList(i,3)=10e-3;
%     elseif peakList(i,1)<=PARAM.nAfferentInPattern+round((PARAM.nAfferent-PARAM.nAfferentInPattern)/2)
%         peakList(i,3)=5e-3;
%     else
%         peakList(i,3)=10e-3;
%     end
% end

% %time compression
% timedLog('Time compression...')
% load(['../data/timeCompression.' sprintf('%03d',PARAM.randState) '.mat'])
% for i=1:size(peakList,1)
%     peakList(i,2) = timeCompression(ceil(peakList(i,2)/PARAM.dt));
% end
% timedLog(['Done'])

plotRate = false;
if PARAM.peakedRate
    plotRate =  PARAM.nAfferent==1 || (  PARAM.nAfferent<=10 && PARAM.T<5); % criteria for plotting rates
    timedLogLn(['PEAKS TO SPIKES - RAND # ' int2str(PARAM.randState)])
    rand('state',PARAM.randState)
    if PARAM.rCoef
        load(['../data/rCoef.' sprintf('%03d',PARAM.randState) '.mat'])
    else
        rCoef = ones(1,floor(PARAM.T/PARAM.dt));
    end
    [spikeList,r] = peak2spike(peakList,plotRate,rCoef);
%     [spikeList,r] = peak2spike(peakList,true,rCoef);
    if plotRate
        disp(['Max r.dt = ' num2str(max(r(:))*PARAM.dt)])
    end
    timedLog(['Done'])
else
    spikeList = peakList(:,1:2);
end

% save spike list (in various files)
if PARAM.T>10
    N=length(spikeList);
    n = 10^6;
    nsl = ceil(N/n);
    for i=1:nsl
        sl = spikeList((i-1)*n+1:min(n*i,N),:);
        sl(:,1) = sl(:,1)-1; % python indexes start at 0
        disp(['saving spikeList.' sprintf('%03d',PARAM.randState) '.' sprintf('%03d',i) '.mat'])
        save(['../data/spikeList.' sprintf('%03d',PARAM.randState) '.' sprintf('%03d',i) '.mat'],'sl')
    end
end

if PARAM.nAfferent==1 && plotRate
    timeCompression=zeros(1,round(PARAM.T/PARAM.dt));
    if patternPeriod(1,1)>0    
        startTime = 0;
        endTime = patternPeriod(1,1);

        idx = round(startTime/PARAM.dt)+1:round(endTime/PARAM.dt);
        timeCompression(idx) =  startTime + (endTime-startTime) * cumsum((r(idx)).^-1)/sum(r(idx).^-1);
    end
    for p=1:size(patternPeriod,1)
        startTime = patternPeriod(p,1);
        endTime = patternPeriod(p,2);

        idx = round(startTime/PARAM.dt)+1:round(endTime/PARAM.dt);
        timeCompression(idx) =  startTime + (endTime-startTime) * cumsum((r(idx)).^-1)/sum(r(idx).^-1);
    %     idx(1)
    %     idx(end)
    %     timeCompression(600)

        if patternPeriod(p,2)<PARAM.T
            startTime = patternPeriod(p,2);
            if p<size(patternPeriod,1)            
                endTime = patternPeriod(p+1,1);
            else
                endTime = PARAM.T;
            end
            idx = round(startTime/PARAM.dt)+1:round(endTime/PARAM.dt);
    %         idx(1)
    %         idx(end)
            timeCompression(idx) =  startTime + (endTime-startTime) * cumsum((r(idx)).^-1)/sum(r(idx).^-1);
    %         return
        end
    end
    alpha = 0.25;
    k=(alpha*mean(r)-min(r))/(1-alpha);
    rCoef = (r+k)/(mean(r)+k);
    save(['../data/timeCompression.' sprintf('%03d',PARAM.randState) '.mat'],'timeCompression')
    save(['../data/rCoef.' sprintf('%03d',PARAM.randState) '.mat'],'rCoef')
    
end
