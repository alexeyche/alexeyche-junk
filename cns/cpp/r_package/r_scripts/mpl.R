
require(Rdnn)

v = RProto$new("/home/alexeyche/dnn/ts/riken_14chan_3LRR.pb")$read()

conf = list(
    filters_num=64,
    batch_size=1000,
    jobs=1,
    learn_iterations=50,
    learning_rate=0.05,
    seed=1,
    learn=FALSE
)

mpl = RMatchingPursuit$new(conf)

start_f = mpl$getFilter()
for(ep in 1:50) {
    inp = v[["values"]][[1]][1:1000]
    spikes = mpl$run(inp)
}

#gr_pl(mpl$getFilter())

x_d = mpl$restore(spikes)[1:200]
x = inp[1:200]
plot(x, ylim=c(min(x_d,x), max(x,x_d)), type="l")
lines(x_d,col="red")

x = c()
y = c()
cex = c()
for(sp in spikes) {
    x = c(x, sp$t)
    y=c(y, sp$fi)
    cex = c(cex, sp$s)
    
}
xyplot(y ~ x, list(x = x, y = y), cex=cex*10,  col = "black")
