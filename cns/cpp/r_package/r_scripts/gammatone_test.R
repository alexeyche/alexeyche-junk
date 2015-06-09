
require(Rdnn)

g = Vectorize(function(time, a, n, b, f, phi) {
    a * time^(n-1) * exp(-2*pi*b*time) * cos(2*pi*f*time + phi)
}, c("time"))

L = 100
fnum = 24
n = 6 
hb=17.33333 
b=0.1
Tmax=1.2
sampling_rate = L/Tmax     

time = seq(0.0,Tmax, length.out=L)
a = 10.0
#n = 4
#b = 2

f = exp(seq(0,log(hb), length.out=fnum))
#f = seq(0,hb, length.out=fnum)

phi = 0
gans = t(sapply(f, function(ffreq) g(time, a, n, b, ffreq, phi)))

plot(gans[fnum, ], type="l")

v = gans[fnum, ]
freq = seq(0, sampling_rate/2, length.out=floor(length(v)/2))
sp = spectrum(v, plot=FALSE)
plot(freq, log(sp$spec), type="l")

RProto$new(path.expand("~/cpp/build/filter.pb"))$write(list(gans), "DoubleMatrix")