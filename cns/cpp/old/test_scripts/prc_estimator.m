%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                               %    
%  Estimating phase-resetting curves (PRC) from real neurons    %
%                                                               %                
%  Written by Roberto Fernandez Galan, November 2004            %
%                                                               %
%  The details and philosophy of this method are published in   %
%  R.F.Galan et al.(2005), Physical Review Letters 94, 158101   %
%                                                               %
%  Complementary information available at:                      %
%                                                               %
%      http://www.case.edu/med/galanlab/prc/prc.html            %
%                                                               %        
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear
close all
clc

dt = 0.1;       % sampling interval in ms
thres = 10;     % spike threshold in mV
    
% reading experimental data

    x = load('prc_input.txt');  % input pulses in pA
    y = load('prc_output.txt'); % neural response (membrane potential) in mV
    
    [m,n] = size(y);
    x = reshape(x,1,m*n);
    y = reshape(y,1,m*n);

% detecting input pulses and spikes

x = x - median(x);
x = (x > 0.01);
x = diff(x) > 0;

spk = (y > thres);
spk = diff(spk) > 0;
spk = find(spk == 1);

% estimating average period and frequency

T0 = median(diff(spk))*dt;

w0 = 2*pi/T0
f0 = 1000/T0

theta = zeros(1,spk(end));
dtheta = zeros(1,spk(end));

% calculating the instantaneous phase

for s = 1:length(spk)-1
    
    for t = spk(s):spk(s+1)
        
        theta(t) = 2*pi*(t-spk(s))/(spk(s+1)-spk(s));
        dtheta(t) = 2*pi/(dt*(spk(s+1)-spk(s)));
        
    end
    
end
theta
dtheta
exit
% extracting the shape of the action potential (spike) for the figures

spkshape = y(spk(10):spk(11));
         
            [maxspk,ind] = max(spkshape);
            phasemax = 2*pi*ind/length(spkshape);
            spkshape = [spkshape(ind+1:end) spkshape(1:ind+1)];

% shifting the phase, so that zero corresponds to the maximum of the action potential
            
            theta = theta(spk(1):spk(end));
            theta = mod(theta - phasemax,2*pi);

% normalizing the angular velocity            
            
dtheta = dtheta(spk(1):spk(end))/w0-1;

% reducing dimensionality before fitting

input = x(spk(1):spk(end));
k = find(input > 0.5);
theta = theta(k);
dtheta = dtheta(k);
input = input(k);

% least-square fitting to a Fourier expansion

A = [ones(size(theta)).*input; sin(theta).*input; cos(theta).*input;...
                       sin(2*theta).*input; cos(2*theta).*input;
                       sin(3*theta).*input; cos(3*theta).*input];                      
A = A';  
fourcoeff = A\dtheta';

% reconstructing the PRC through its Fourier components

phi = [0:0.1:2*pi];
prc = [ones(size(phi)); sin(phi); cos(phi); sin(2*phi); cos(2*phi);...
                        sin(3*phi); cos(3*phi)]; 

prcodd = [sin(phi); sin(2*phi); sin(3*phi)];        
prceven = [ones(size(phi)); cos(phi); cos(2*phi); cos(3*phi)]; 
                          
prc = fourcoeff'*prc;
prcodd = fourcoeff([2 4 6])'*prcodd;
prceven = fourcoeff([1 3 5 7])'*prceven;


% plotting results

prc = [prc prc(1)];
prcodd = [prcodd prcodd(1)];
phi = [phi phi(end)+0.1];

figure(1)
plot(phi,prc/max(prc),'linewidth',2)
hold on
plot(phi,prcodd/max(prcodd),'g','linewidth',2)
plot(2*pi*[0:1:length(spkshape)-1]/length(spkshape),spkshape/max(spkshape),'r','linewidth',2)
plot(phi,zeros(1,length(phi)),'k--')
xlabel('phase angle (rad)','fontsize',14)
title('experimentally estimated phase-resetting curve','fontsize',14)
h = legend('PRC','odd PRC','spike cycle',2);
set(h,'fontsize',14,'fontname','helvetica')
set(gca,'xtick',[0 1/2 1 3/2 2]*pi)
set(gca,'fontname','symbol','fontsize',14,'xticklabel',{'  0','p/2',' p ','3p/2','2p'})
axis tight

figure(2)
[theta,ind] = sort(theta);
dtheta = dtheta(ind);
Prc = prc/max(prc);
plot(2*pi*[0:1:length(spkshape)-1]/length(spkshape),-spkshape/min(spkshape)+0.5,'b','linewidth',2)
hold on
plot(phi,Prc,'r','linewidth',2)
ylabel('normalized phase-shift','fontsize',14)
xlabel('phase angle (rad)','fontsize',14)
title('experimentally estimated phase-resetting curve','fontsize',14)
h = legend('spike cycle','estimated PRC',2);
set(h,'fontsize',14,'fontname','helvetica')
set(gca,'xtick',[0 1/2 1 3/2 2]*pi)
set(gca,'fontname','symbol','fontsize',14,'xticklabel',{'  0','p/2',' p ','3p/2','2p'})
axis([0 2*pi -0.51 1.1]) 
