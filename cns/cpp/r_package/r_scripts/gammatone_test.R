
require(Rdnn)

g = Vectorize(function(time, a, n, b, f, phi) {
    a * time^(n-1) * exp(-2*pi*b*time) * cos(2*pi*f*time + phi)
}, c("time"))

sampling_rate = 100
Tmax=1
fnum = 50
#hb = 20 # Hz

time = seq(0.0,Tmax, length.out=Tmax*sampling_rate)
a = 10.0
#n = 4
#b = 2
f = exp(seq(0,log(hb), length.out=fnum))
#f = seq(0,hb, length.out=fnum)

phi = 0
gans = t(sapply(f, function(ffreq) g(time, a, n, b, ffreq, phi)))

plot(gans[50, ], type="l")

v = gans[50, ]
freq = seq(0, sampling_rate/2, length.out=floor(length(v)/2))
sp = spectrum(v, plot=FALSE)
#plot(freq, log(sp$spec), type="l")

RProto$new(path.expand("~/cpp/build/filter.pb"))$write(list(gans), "DoubleMatrix")