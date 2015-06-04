
require(Rdnn)


v = RProto$new("/home/alexeyche/dnn/ts/riken_14chan_3LRR.pb")$read()

conf = list(
    filters_num=32,
    batch_size=100000,
    jobs=4,
    learn_iterations=100,
    learning_rate=0.5,
    seed=1,
    threshold=0.05,
    learn=TRUE,
    noise_sd=0.0
)

norm = function(x) {
    x/sqrt(sum(x^2))
}

err = function(r, x) {
    sum((r-x[1:length(r)])^2)
}

mpl = RMatchingPursuit$new(conf)
#set.seed(1)
#f=matrix(norm(rnorm(32*100)), nrow=32, ncol=100)
#mpl$setFilter(f)


max_ep = 101

start_f = mpl$getFilter()

setwd(path.expand("~/cpp/build"))
for(ep in 1:max_ep) {
    inp = v[["values"]][[1]] #[1:10000]
#     if((ep %% 10 == 0)||((ep-1)%% 10 == 0)) {
#         conf$noise_sd = 0.01
#         mpl$setConf(conf)
#     } else {
#         conf$noise_sd = 0.0
#         mpl$setConf(conf)
#     }
    ret = mpl$run(inp)
    spikes = ret$spikes  
    restored = mpl$restore(spikes)
    cat("Error: ", err(restored, inp), "\n")    
}
conf$noise_sd = 0.0
mpl$setConf(conf)

end_f = mpl$getFilter()
gr_pl(end_f)

x_d = mpl$restore(spikes)[1:500]
x = inp[1:500]
plot(x, ylim=c(min(x_d,x), max(x,x_d)), type="l")
lines(x_d,col="red")



