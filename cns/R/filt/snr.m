% SNR test 
clc; clear all; close all; 

% Units 
MHz = 1e6; 

% Variables 
Fs = 10*MHz;                    % Sample rate 
Ts = 1/Fs;                      % Sample period 
L = 4096;                       % Sample record length 
m = 0:L-1;                      % Sample vector 
f_if = 1*MHz; 

omega_if = 2*pi*f_if;           % Analog IF frequency [rad/s] 
W_if = 2*pi*f_if/Fs;            % Digital IF frequency [rad/s] 
theta_n = deg2rad(30);          % Phase shift [radians] 
SNR = 40;                       % Signal to noise ratio of message waveform x[m] [dB] 

g0 = cos(W_if.*m - theta_n);     % Generate ideal signal and phase shift 
g1 = awgn(g0,SNR,'measured');   % Add white Gaussian noise to signal 

% SNR estimate of signal to verify awgn is properly used 
% Signal power (Integrate over samples of original signal, divide by record length) 
p_signal = sum(abs(g0).^2,2)./length(g0)     
% Signal noise (Calculate variance of noise) 
p_noise = std(g1 - g0,0,2).^2         
% SNR calculation in time 
SNR_time = 10*log10(p_signal./p_noise) 


% Perform FFT of g0 
NFFT = 2^nextpow2(L);           % Next power of 2 from length of y 
%NFFT = 1024; 
G0 = fft(g0,NFFT,2)/L;          % FFT 
G0 = fftshift(G0,2);            % Shift spectrum to [-Fs/2,Fs/2] 
G0_dB = 20*log10(abs(G0));      % Convert to dB 

% Perform FFT of g1 
G1 = fft(g1,NFFT,2)/L;          % FFT 
G1 = fftshift(G1,2);            % Shift spectrum to [-Fs/2,Fs/2] 
G1_dB = 20*log10(abs(G1));      % Convert to dB 

f = Fs/2*linspace(-1,1,NFFT);   % Generate frequency list 

% SNR estimate using spectrum of signal to verify awgn is properly used 
% Signal power (Integrate over samples of original signal, divide by record length) 
p_signal_f = sum(abs(G0).^2,2)./length(G0) 
% Signal noise (Calculate variance of noise) 
p_noise_f = std(G1 - G0,0,2).^2         
p_noise_f1 = sum(abs(G1 - G0).^2,2) 
% SNR calculation 
SNR_freq = 10*log10(p_signal_f./p_noise_f) 

% Attempt to calculate SNR from spectrum, subtracting processing gain 
SNR_freq1 = max(G1_dB)- 10*log10(p_noise_f) - 10*log10(L/2) 
SNR_freq2 = max(G1_dB)- 10*log10(p_noise_f1) - 10*log10(L/2) 

% Plot time-domain signal 
figure; 
plot(m,g0,m,g1) 
hold on; 
%stem(m,g1,'r') 
xlabel('Sample Number') 
ylabel('Value') 
x0 = Fs/(omega_if/(2*pi))*4; 
xlim([0 x0]) 
legend('Signal','Signal + Noise') 
hold off; 

% Plot spectrum magnitude. 
figure; 
plot(f./MHz,G0_dB,f./MHz,G1_dB) 
title('Spectrum Magnitude of g0(t) and g1(t)') 
        
xlabel('Frequency (MHz)') 
ylabel('Magnitude') 
legend('Signal','Signal + White Noise') 

20*log10(mean(abs(G1))) + 10*log10(L/2) + SNR 
max(G1_dB) - (20*log10(mean(abs(G1))) + 10*log10(L/2)) 
