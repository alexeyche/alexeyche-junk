
require(Rdnn)

prast = function(spikes) {
    x = c()
    y = c()
    cex = c()
    for(sp in spikes) {
        x = c(x, sp$t)
        y=c(y, sp$fi)
        cex = c(cex, sp$s)
        
    }
    xyplot(y ~ x, list(x = x, y = y), cex=cex*10,  col = "black")
}

v = RProto$new("/home/alexeyche/dnn/ts/riken_14chan_3LRR.pb")$read()

conf = list(
    filters_num=64,
    batch_size=1000,
    jobs=1,
    learn_iterations=100,
    learning_rate=1,
    seed=1,
    threshold=0.1,
    learn=FALSE,
    noise_sd=0.0
)
norm = function(x) {
    x/sqrt(sum(x^2))
}
bd = "/home/alexeyche/prog/alexeyche-junk/cns/cpp/build"
gbd = function(ff) {
    RProto$new(sprintf("%s/%s",bd,ff))$read()[[1]]
}
err = function(r, x) {
    sum((r-x[1:length(r)])^2)
}

mpl = RMatchingPursuit$new(conf)
mpl$setFilter(gbd("filter.pb"))


max_ep = 100

start_f = mpl$getFilter()

setwd(path.expand("~/cpp/build"))

for(ep in 1:max_ep) {
    inp = v[["values"]][[1]][1:1000]
    ret = mpl$run(inp)
    spikes = ret$spikes  
    cat("accum error: ", ret$accum_error, "\n")
    
    rest_a = mpl$restore(spikes)
    
    rest_b = gbd("restored.pb")
}

end_f = mpl$getFilter()
gr_pl(end_f)

x_d = mpl$restore(spikes)[1:500]
x = inp[1:500]
plot(x, ylim=c(min(x_d,x), max(x,x_d)), type="l")
lines(x_d,col="red")



