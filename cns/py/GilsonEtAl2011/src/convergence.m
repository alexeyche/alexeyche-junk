n=1;
% figure
leg={};

windowSize = 1;

conv0 = 0.0350/0.1125/2;
conv0LIF = .7/2.8/2;

randState = 0;
filepath = '../data/';
dirlist = dir([filepath 'weight.' sprintf('%03d',randState)  '.*.mat']);

clear conv
conv = conv0;
for f=1:length(dirlist)
    load([filepath dirlist(f).name])
%     figure
%     hist(weight(:,n))
    conv(f+1)=mean(abs(weight(:,n)-(weight(:,n)>.5)));
end

conv = filter(ones(1,windowSize)/windowSize,1,conv);
conv = conv(windowSize:end);
plot(50*([0:length(conv)-1]+windowSize-1),conv,'.-')

leg{end+1} = 'S';
hold on

axis([0 1 0 .2])
axis 'auto x'
