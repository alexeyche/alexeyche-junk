

gauss_f = function(x, mu, dev) { 
    #    (1/(2*pi*(dev^2))) * 
    exp( -((x-mu)^2)/(2*dev^2))
}

filt = gauss_f(seq(0,20), 10 ,5) 

max_val = max(data$EEGDATA)
min_val = min(data$EEGDATA)
normalize = function(x, min_val, max_val) {
    1 - (max_val-x)/(max_val-min_val) 
}


chan = 1
ex = 1
input = normalize(data$EEGDATA[chan,,ex], min_val, max_val)

bsa_alg = function(input, filt, threshold) {
    sp_seq = NULL
    err1_seq = NULL
    err2_seq = NULL
    for(i in 1:length(input)) {
        error1 = 0
        error2 = 0
        for(j in 1:length(filt)) {
            if( (i+j-1) <= length(input) ) {
                error1 = error1 + abs( input[i+j-1] - filt[j])
                error2 = error2 + abs( input[i+j-1] )           
            }
        }
        if( error1 <= (error2 - threshold)) {
            sp_seq = c(sp_seq, 1)
            for(j in 1:length(filt)) {
                if( (i+j-1) <= length(input) ) {
                    input[i+j-1] = input[i+j-1] - filt[j] 
                }
            }
        } else {
            sp_seq = c(sp_seq, 0)
        }
        err1_seq = c(err1_seq, error1)
        err2_seq = c(err2_seq, error2)
    }
    return( list(sp_seq, err1_seq, err2_seq) )
}

calc_snr = function(g0, g1) {
    p_signal = sum(abs(g0)^2)/length(g0)
    p_noise = sd(g1-g0)^2
    
    snr_time = 10*log10(p_signal/p_noise)
    
    G0 = fft(g0)/length(g0)
    G0 = fftshift(G0)
    G0_dB = 10*log10(abs(G0))
    
    G1 = fft(g1)/length(g0)
    G1 = fftshift(G1)
    G1_dB = 10*log10(abs(G1))
    
    Fs = 1000 # Hz, sampling rate
    f = Fs/2*seq(-1,1,length.out=length(g0))
    
    p_signal_f = sum(abs(G0)^2)/length(G0)
    p_noise_f = sd( abs(G1-G0)^2 )
    p_noise_f1 = sum( abs(G1-G0)^2 )
    
    snr_freq = 10*log10(p_signal_f/p_noise_f)
    return(snr_freq)
}

calc = function(mod, threhold) {
    c(sp_seq, err1, err2) := bsa_alg(input, filt*mod, threshold)
    s_est = fftfilt(filt*mod, sp_seq)
    return(list(sp_seq, err1, err2, s_est))
}


thresholds = seq(0.5,1.0, length.out=50)
mods = seq(0, 0.2, length.out=50)
snr = NULL
for(mod in mods) {
    snr_mod = NULL
    for(threshold in thresholds) {
        c(sp_seq, err1, err2, s_est) := calc(mod, threhold)
        snr_freq = calc_snr(input, s_est)
        snr_mod = c(snr_mod, snr_freq)
    }
    snr = rbind(snr, snr_mod)
}

ind = which(snr == max(snr), arr.ind=TRUE)
mod = mods[ind[1]]
threshold = thresholds[ind[2]]
c(sp_seq, err1, err2, s_est) := calc(mod, threhold)

Tmax=300
v = input[1:Tmax]
tx = seq(0,Tmax,length.out=Tmax)
sp = list(which(sp_seq[1:Tmax]==1))
err1s = err1[1:Tmax]
err2s = err2[1:Tmax]
s_est_s = s_est[1:Tmax]

pl = xyplot(v+s_est_s + (v-s_est_s)~tx, type="l", xlab="time", col=c("black","blue", "red"),xlim=c(0,Tmax))
err_pl = xyplot(err1s+err2s+(err1s - (err2s-threshold))~tx, type="l", xlab="time", col=c("blue","red", "black"),xlim=c(0,Tmax))
sp_plot = prast(sp)
print(pl, position=c(0, 0.66, 1, 1), more=TRUE)
print(sp_plot, position=c(0, 0.33, 1, 0.66), more=TRUE)
print(err_pl, position=c(0, 0.0, 1, 0.33), more=FALSE)
