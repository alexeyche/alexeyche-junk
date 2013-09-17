timedLogLn(['GENERATE PEAKS - RAND # ' int2str(PARAM.randState)])

switch PARAM.pattern_type
    case PARAM.ACADEMIC
        pattern = [ 1:PARAM.nAfferentInPattern ; [1:PARAM.nAfferentInPattern]/PARAM.nAfferentInPattern*PARAM.patternDuration ]';
    case PARAM.POISSON
        pattern = poisson(PARAM.nAfferentInPattern,PARAM.patternDuration,1/PARAM.meanFreq);
%         pattern = poisson(PARAM.nAfferentInPattern,PARAM.patternDuration,1/17);
    case PARAM.N1N2
       % '2 cluster' N1 , N2
%        N1 = round(1/3*PARAM.nAfferentInPattern);
       N1 = round(.4*PARAM.nAfferentInPattern);
       % 1 spikes/afferent
       pattern = [ [1:PARAM.nAfferentInPattern]' , [ randn(N1,1)*5e-3+15e-3 ; randn(PARAM.nAfferentInPattern-N1,1)*5e-3+35e-3 ]  ];
%        pattern = [ [1:PARAM.nAfferentInPattern]' , [ ones(N1,1)*15e-3 ; ones(PARAM.nAfferentInPattern-N1,1)*5e-3 ]  ];
    %    % 2 spikes/afferent
    %    pattern = [ floor([1:.5:PARAM.nAfferentInPattern+.5])' , [ randn(2*N1,1)*5e-3+15e-3 ; randn(2*(PARAM.nAfferentInPattern-N1),1)*5e-3+35e-3 ]  ];
    case PARAM.FIG
        pattern = [1 .015; 2 .005];
        if PARAM.nAfferent>2
            % illustration
            pattern = [pattern; 3 .020; 3 .04; 3 .045];  
        end
    case PARAM.PSTH
        pattern = [1 .100; 1 .150; 1 .300; 1 .380; 1 .400 ];
end

% save pattern values for Brian
% pattern values (for Brian) = first spike latency
realValuedPattern = Inf * ones(1,PARAM.nAfferentInPattern);
for i=1:size(pattern,1)
    realValuedPattern(pattern(i,1)) = min(realValuedPattern(pattern(i,1)),pattern(i,2)/PARAM.patternDuration);
end
realValuedPattern(realValuedPattern==Inf)=NaN;
save(['../data/realValuedPattern.' sprintf('%03d',PARAM.randState) '.mat'],'realValuedPattern')
   
if PARAM.peakedRate
    % add sigma
    pattern = [pattern, PARAM.speak0 + PARAM.speak1 * randn(size(pattern,1),1)];
%     pattern = [pattern, [ ones(N1,1)*10e-3 ; ones(PARAM.nAfferentInPattern-N1,1)*5e-3 ] ];
    % add rho
    pattern = [pattern, PARAM.rpeak0 + PARAM.rpeak1 * randn(size(pattern,1),1)];
end
% save just in case
save(['../data/pattern.' sprintf('%03d',PARAM.randState) '.mat'],'pattern')
% sort pattern
pattern = sortrows(pattern,2);

if PARAM.peakedRate
    peakList = zeros(round(1.01*PARAM.nAfferent*PARAM.meanFreq*PARAM.T),4);
else
    peakList = zeros(round(1.01*PARAM.nAfferent*PARAM.meanFreq*PARAM.T),2);
end
cursor=1;

tmin=0;
for p=1:length(PARAM.posPattern)

    % part preceding pattern (if any) : [tmin;(PARAM.posPattern(p)-1)*PARAM.patternDuration]
    tmp = sortrows(poisson(PARAM.nAfferent,(PARAM.posPattern(p)-1)*PARAM.patternDuration-tmin,1/PARAM.meanFreq),2);
    peakList(cursor:cursor+size(tmp,1)-1,1:2) = [ tmp(:,1) , tmp(:,2)+tmin];
    if PARAM.peakedRate
        peakList(cursor:cursor+size(tmp,1)-1,3:4) = [PARAM.speak0 + PARAM.speak1 * randn(size(tmp,1),1), PARAM.rpeak0 + PARAM.rpeak1 * randn(size(tmp,1),1)];
    end
    cursor = cursor+size(tmp,1);

    tmin = (PARAM.posPattern(p)-1)*PARAM.patternDuration;

    % pattern period [(PARAM.posPattern(p)-1)*PARAM.patternDuration;PARAM.posPattern(p)*PARAM.patternDuration]
    % pattern part
    tmp = pattern;
    tmp(:,2) = tmp(:,2)+tmin;
    if PARAM.deletion > 0 % delete pattern peaks, and compensate with random (poisson) peaks 
        tmp = tmp( rand(1,size(tmp,1)) > PARAM.deletion, : );
        compensate = poisson(PARAM.nAfferentInPattern,PARAM.patternDuration,1/(PARAM.meanFreq*PARAM.deletion));
        compensate(:,2) = compensate(:,2)+tmin;
        if PARAM.peakedRate
            % add sigma
            compensate = [compensate, PARAM.speak0 + PARAM.speak1 * randn(size(compensate,1),1)];
            % add rho
            compensate = [compensate, PARAM.rpeak0 + PARAM.rpeak1 * randn(size(compensate,1),1)];
        end
        tmp = [tmp ; compensate];
    end
%     peakList(cursor:cursor+size(tmp,1)-1,:)=tmp;
%     cursor = cursor+size(tmp,1);
    
    % distractor part
    dist = poisson(PARAM.nAfferent-PARAM.nAfferentInPattern,PARAM.patternDuration,1/PARAM.meanFreq);
    dist(:,2) = dist(:,2)+tmin;
    dist(:,1) = dist(:,1)+PARAM.nAfferentInPattern;
%     peakList(cursor:cursor+size(tmp,1)-1,1:2) = [ tmp(:,1)+PARAM.nAfferentInPattern , tmp(:,2)+tmin];
    if PARAM.peakedRate
        dist(1:end,3:4) = [ PARAM.speak0 + PARAM.speak1 * randn(size(dist,1),1), PARAM.rpeak0 + PARAM.rpeak1 * randn(size(dist,1),1) ];
    end
    
    tmp = sortrows([tmp;dist],2);
    
    peakList(cursor:cursor+size(tmp,1)-1,:)=tmp;
    cursor = cursor+size(tmp,1);

    tmin = PARAM.posPattern(p)*PARAM.patternDuration;

    if mod(p,round(PARAM.T/5*PARAM.patternFreq/PARAM.patternDuration))==0
        disp(['t=' num2str(tmin) 's'])
    end
    
end
% part after last pattern (if any) [ tmin;PARAM.T ]
tmp = sortrows(poisson(PARAM.nAfferent,PARAM.T-tmin,1/PARAM.meanFreq),2);
peakList(cursor:cursor+size(tmp,1)-1,1:2) = [ tmp(:,1) , tmp(:,2)+tmin ];
if PARAM.peakedRate
    peakList(cursor:cursor+size(tmp,1)-1,3:4) = [ PARAM.speak0 + PARAM.speak1 * randn(size(tmp,1),1), PARAM.rpeak0 + PARAM.rpeak1 * randn(size(tmp,1),1) ];
end
cursor = cursor+size(tmp,1);

if size(peakList,1) > 10^3 && cursor-1==size(peakList,1);
    warning('Increase peakList array size at initialization for better performance')
end
if size(peakList,1) > 10^3 && cursor<.5*size(peakList,1)
    warning('Decrease peakList array size at initialization for better performance')
end

% remove non used values
peakList(cursor:end,:)=[];
% peakList=peakList(1:cursor-1,:);

if PARAM.peakedRate
    % check for negative values
    if min(peakList(:,3)) < 0
        warning('Negative sigma peak. Taking absolute value')
        peakList(:,3) = abs(peakList(:,3));
    end
    if min(peakList(:,4)) < 0
        warning('Negative rho peak. Taking absolute value')
        peakList(:,4) = abs(peakList(:,4));
    end
end

% % sort list
% peakList = sortrows(peakList,2);

disp(['Mean peak frequency = ' num2str(size(peakList,1)/PARAM.nAfferent/PARAM.T) ' Hz' ] )
timedLog(['Done'])
