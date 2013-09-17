
% This script is useful to generate spike trains with bursts,
% and thus Fano factor > 1 (see Supplemental Information Section S1.2 and Fig. S3)

slpath = '../data/';
randState = 0;
randStateOut = 1;
p = 0.25;
k = 1;
disp(['FF = 1+k*(k+1)*p/(1+k*p) = ' num2str(1+k*(k+1)*p/(1+k*p))])
dt = 1e-3;


rand('state',randState)

files = dir([slpath 'spikeList.' sprintf('%03d',randState) '.*.mat']);
for f=1:length(files)
    clear sl
    disp(files(f).name)
    load([slpath files(f).name]);
    
    newsl = zeros(round(1.1*length(sl)),2);
    cursor=1;
    for i=1:length(sl)
        if rand < 1/(1+k*p) % event is not discarded
            if rand < p % burst
                newsl(cursor:cursor+k,1) = sl(i,1); 
                newsl(cursor:cursor+k,2) = sl(i,2)-k/2*dt:dt:sl(i,2)+k/2*dt;                
                cursor = cursor+k+1;
            else % regular spike
                newsl(cursor,:) = sl(i,:); 
                cursor = cursor+1;
            end
        end
    end
    
    newsl(cursor:end,:) = [];
    sl = sortrows(newsl,2);
    save('-V7',[ slpath 'spikeList.' sprintf('%03d',randStateOut) '.' sprintf('%03d',f) '.mat'],'sl')
    
end
disp('Done.')
