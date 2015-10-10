
require(Rdnn)
require(rjson)


err = function(r, x) {
    sum((r-x[1:length(r)])^2)
}

args <- commandArgs(trailingOnly = FALSE)

EP = convNum(Sys.getenv('EP'), -1)
pfx_f = function(s) s
if(EP>=0) {
    pfx_f = function(s) sprintf("%d_%s", EP, s)
}
WD = convStr(
    Sys.getenv("WD"), 
    sprintf("~/dnn/mpl/runs/%s", system("ls -t ~/dnn/mpl/runs | head -n 1", intern=TRUE))
)

#conf = fromJSON(file=sprintf("%s/mpl.json", WD))[[1]]

DIM = convNum(Sys.getenv("DIM"), 1)

input = RProto$new("/home/alexeyche/dnn/ts/riken_14chan_3LRR.pb")$read()

v = input[["values"]][[DIM]]

setwd(path.expand(WD))

f = RProto$new(pfx_f("filter.pb"))$read()[[1]]

if(length(grep("RStudio", args))==0) {
    test_pic = pfx_f("spikes.png")
    png(test_pic, width=1024, height=768)
}

convSpikeToListOfNeurons = function(spikes) {
    net = blank_net(max(unique(spikes$fi))+1)
    for(i in 1:length(spikes$fi)) {
        net[[ spikes$fi[i]+1 ]] = c(net[[ spikes$fi[i]+1 ]], spikes$t[i])
    }
    return(net)
}

spikes = RProto$new(pfx_f("spikes.pb"))$read()
prast(spikes,T0=0,Tmax=2000)

net = convSpikeToListOfNeurons(spikes)
rates = 1000*sapply(net, length)/length(v)
plot(rates)

if(length(grep("RStudio", args))==0) dev.off()

r = RProto$new(pfx_f("restored.pb"))$read()[["values"]][[1]]

idx = 1:1000 + 0
x = v[idx]
x_d = r[idx]
delta = x - x_d
plot(x, ylim=c(min(x_d,x), max(x,x_d)), type="l")
lines(x_d,col="green")
lines(delta, col="red")

v = v[1:length(r)]
sampling_rate = 1000
freq = seq(0, sampling_rate/2, length.out=floor(length(v)/2)+1)

hb = 300 # Hz
hb_i = min(which(hb < freq))
bound = function(s) {
    s[1:hb_i]
}


if(length(grep("RStudio", args))==0) {
    test_pic = pfx_f("filter.png")
    png(test_pic, width=1024, height=768)
}


smooth_level = c(3,10)
signal_spectra = spec.pgram(v, spans=smooth_level, taper=0.0, plot=FALSE)
restored_spectra = spec.pgram(r, spans=smooth_level, taper=0.0, plot=FALSE)
noise_spectra = spec.pgram(v-r, spans=smooth_level, taper=0.0, plot=FALSE)

log_sig_spec = log(signal_spectra$spec)
log_rest_spec = log(restored_spectra$spec)
snr = signal_spectra$spec/noise_spectra$spec
inf = sum(log2(1+snr))/sampling_rate
freq = sampling_rate*signal_spectra$freq
log_noise_spec = log(noise_spectra$spec)

log_sig_spec = bound(log_sig_spec)
log_rest_spec = bound(log_rest_spec)
log_noise_spec = bound(log_noise_spec)
snr = bound(snr)
freq = bound(freq)

plot(
    freq,
    log_sig_spec,
    type="l", col="black", ylab="Power Db", axes=FALSE, main=sprintf("Spectra and SNR, I = %3.5f",inf), xlab="Freq Hz"
)
axis(2, ylim=c(min(log_sig_spec), max(log_sig_spec)),col="black",las=1) 
par(new=TRUE)

plot(
    freq,
    log_rest_spec,
    type="l", col="red", ylab="", axes=FALSE, xlab=""
)

par(new=TRUE)
plot(
    freq, 
    snr,
    type="l", col="green", axes=FALSE, xlab="", ylab=""
)


axis(4, ylim=c(0,max(snr)), col="green", las=1)
axis(1, ylim=c(min(freq), max(freq)))

if(length(grep("RStudio", args))==0) dev.off()
cat("Information:", inf,"\n")


