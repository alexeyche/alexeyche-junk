
require(Rdnn)
dim = 1
v = RProto$new("/home/alexeyche/dnn/ts/riken_14chan_3LRR.pb")$read()[[1]]

v = v[[dim]]

fnum = 24
L = 100

err = function(r, x) {
    r = c(r, rep(0, length(x)-length(r)))
    sum((r-x)^2)
}

conf = list(
    filters_num=fnum,
    batch_size=length(v),
    jobs=4,
    learn_iterations=100,
    learning_rate=0.0,
    seed=1,
    threshold=0.07,
    learn=FALSE,
    noise_sd=0.0
)


g = Vectorize(function(time, a, n, b, f, phi) {
    a * time^(n-1) * exp(-2*pi*b*time) * cos(2*pi*f*time + phi)
}, c("time"))

sampling_rate = 100
Tmax=1


hb = 20 # Hz
a = 10.0
n = 4
b = 2


fb = function(fnum, hb, a, n, b, Tmax, sampling_rate) {
    time = seq(0.0,Tmax, length.out=L)
    phi = 0
    
    f = exp(seq(0,log(hb), length.out=fnum))
    return(t(sapply(f, function(ffreq) g(time, a, n, b, ffreq, phi))))
}
# seq_len, min:
# n: 6 hb: 17.33333 b: 0.1 Tmax: 1.2 error:  1.942685 
seq_len = 30

hb_min=1
hb_max=50
n_min=1
n_max=10
b_min = 0.1
b_max = 10
Tmax_min=0.1
Tmax_max = 10

hb_s = seq(hb_min, hb_max, length.out=seq_len)
n_s = seq(n_min, n_max, length.out=seq_len)
b_s = seq(b_min, b_max, length.out=seq_len)
Tmax_s = seq(Tmax_min, Tmax_max, length.out=seq_len)

errs = array(0, c(seq_len, seq_len, seq_len, seq_len))

i = 1
for(n in n_s) {    
    j = 1
    for(hb in hb_s) {
        k = 1
        for(b in b_s) {
            l = 1
            for(Tmax in Tmax_s) {
                hb = hb # Hz
                a = 10.0
                sampling_rate = L/Tmax     
                f = fb(fnum, hb, a, n, b, Tmax, sampling_rate)
                mpl = RMatchingPursuit$new(conf)
                mpl$setFilter(f)
                ret = mpl$run(v)
                restored = mpl$restore(ret$spikes)
                e = err(restored, v)
                errs[i, j, k, l] = e
                cat("n:", n, "hb:", hb, "b:", b, "Tmax:",Tmax,"error: ", e, "\n")
                l = l+1
            }
            k = k+1
        }
        j = j+1
    }
    i = i+1
}

ii = which(min(errs) == errs, arr.ind=TRUE)
n = n_s[ii[1]]
hb = hb_s[ii[2]]
b = b_s[ii[3]]
Tmax = Tmax_s[ii[4]]
cat("n:", n, "hb:", hb, "b:", b, "Tmax:",Tmax,"error: ", min(errs), "\n")