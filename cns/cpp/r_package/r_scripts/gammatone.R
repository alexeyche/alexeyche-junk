

require(Rsnn)
source('ucr_ts.R')

sample_size = 60

c(train, test) := read_ts_file(synth, sample_size, "~/prog/sim/ts")

fb = RGammatoneFB$new()

low_f = 1
high_f = 200
freqs = seq(low_f, high_f, length.out=100)

x = train[[5]]$data

o = fb$calc(x, freqs,2000, 1,1)

mem = do.call(rbind, o$membrane)
gr_pl(t(mem))