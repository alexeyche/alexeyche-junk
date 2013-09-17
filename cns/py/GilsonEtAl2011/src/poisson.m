function sl = poisson(n,T,mu)


sl = zeros(round(1.2*1/mu*T*n),2);
cursor = 1;
for i=1:n
    previous = 0;
    while true
%         new = previous + exprnd(mu);
        new = previous - mu .* log(rand);
        if new<=T
            sl(cursor,:) = [i new];
            cursor = cursor+1;
            previous = new;
        else
            break
        end
    end
%     tmp = cumsum(exprnd(1/rate,1,round(8.0*rate*T)));
%     if ~isempty(tmp)
%         if tmp(end)<T
%             warning('Missing potential spikes. Increase array size.');
%         end
%         for j=1:length(tmp)
%             if tmp(j)<T
%                 sl(cursor,:) = [i,tmp(j)];
%                 cursor = cursor+1;
%             else
%                 break;
%             end
%         end
%     end
end

if size(sl,1)>10^3 && cursor-1==size(sl,1);
    warning('Increase spikeList array size at initialization for better performance')
end
if size(sl,1)>10^3 && cursor<.5*size(sl,1)
    warning('Decrease spikeList array size at initialization for better performance')
end

% remove non used values
sl(cursor:end,:)=[];

% % sort
% sl = sortrows(sl,2);
