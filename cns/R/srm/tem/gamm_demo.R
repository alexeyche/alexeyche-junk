dur = 0.06

Fs = 1000
dt_f = 1/Fs
t_f = seq(dt_f, dur, by=dt_f)

Ns = 2^9
dt = dt_f/Ns
Ft = 1/dt
t = seq(dt, dur, by=dt)
tr_vc = round(0.1*length(t)):round(0.9*length(t))

fmin = 110
fmax = 390
W  = 2*pi*fmax 

mc = floor(floor(t(end)/dt)*(fmax-fmin-1)*dt)
#rand('twister',0); randn('state',0);
c(trds, testds) := read_ts_file(synth, "~/my/sim")
u = trds[[1]]$data 
#u = u((round(0.15*length(t))+1):end-round(0.15*length(t))-1); 

u = u/max(abs(u))
plot(t, u)

np2 = 64 #2^nextpow2(length(u[tr_vc]));
U = fft(u[tr_vc],np2)/length(u[tr_vc])  

f = Ft/2*seq(0,1,length.out=np2/2+1)
subplot(1,2,2);plot(f,2*abs(U(1:np2/2+1)));
axis([0,Fs/2,0,1.1*max(2*abs(U(1:np2/2+1)))]);
xlabel('f (Hz)'); ylabel('U(f)'); 
title('Signal in the Frequency Domain');

%% Filterbank Construction and Filtering of Signal

Nf = 16;              % # of filters / neurons
flen = ceil(.03/dt);  % length of filters
[h,fc,tg,fg] = gammatone(Nf,flen,fmin,fmax,Ft,0);
