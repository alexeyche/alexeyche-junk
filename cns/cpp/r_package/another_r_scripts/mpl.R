
require(Rdnn)

setVerboseLevel(0)

setwd(path.expand("~/dnn/build"))

v = proto.read("~/dnn/ts/synthetic_control_40_len_4_classes_train.pb")

Sys.setenv(MPLMATCH_PLOT_SIZE=2.5)

conf = list(
    filters_num=100,
    filter_size=100,
    batch_size=40000,
    jobs=4,
    learn_iterations=100,
    learning_rate=0.0001,
    seed=1,
    threshold=0.2,
    learn=TRUE,
    noise_sd=0.0
)
mpl = RMatchingPursuit$new(conf)


norm = function(x) {
    x/sqrt(sum(x^2))
}


err = function(r, x) {
    sum((r-x[1:length(r)])^2)
}

low_f = 0.0
high_f = 200
#freqs = seq(low_f, high_f, length.out=conf$filters_num)
freqs = log.seq(low_f, high_f, conf$filters_num)
m = get.gammatones(freqs, 1000)


mpl$setFilter(m)
gr_pl(t(m))

#set.seed(1)
#f = matrix(rnorm(conf$filters_num*conf$filter_size), nrow=conf$filters_num, ncol=conf$filter_size)
#mpl$setFilter(f)

#f = proto.read("filter.pb")[[1]]
#mpl$setFilter(f)


max_ep = 30

start_f = mpl$getFilter()
proto.write("start_filter.pb", list(start_f), "DoubleMatrix")

for(ep in 1:max_ep) {
    inp = v[["values"]][[1]] #[1:10000]
    cat("epoch: ", ep)
    for(i in 1:1) {
        ret = mpl$run(inp)
        spikes = ret$spikes  
        matches = ret$matches  
        restored = mpl$restore(matches)
        if(i == 1) {
            cat(", error: ", err(restored, inp))
            cat(", residuals: ")
        }
        cat(sum(ret$residual^2), " ")
        inp = ret$residual
    }
    cat("\n")
}

conf$noise_sd = 0.0
mpl$setConf(conf)

end_f = mpl$getFilter()
gr_pl(end_f)
proto.write("end_filter.pb", list(end_f), "DoubleMatrix")

spikes$ts_info = v$ts_info
proto.write("spikes.pb", spikes, "SpikesList")

x_d = mpl$restore(matches)[20000+1:1000]
x = v[["values"]][[1]][20000+1:1000]
plot(x, ylim=c(min(x_d,x), max(x,x_d)), type="l")
lines(x_d,col="red")



#v = proto.read("~/dnn/ts/synthetic_control_40_len_4_classes_train.pb")
#inp = v$values[[1]]
