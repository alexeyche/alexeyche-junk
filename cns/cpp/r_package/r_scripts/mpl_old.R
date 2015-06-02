
require(Rdnn)

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

bd = "/home/alexeyche/prog/alexeyche-junk/cns/cpp/build"
gbd = function(ff) {
    RProto$new(sprintf("%s/%s",bd,ff))$read()[[1]]
}


mpl = RMatchingPursuit$new(conf)
set.seed(1)
f=matrix(rnorm(64*100), nrow=64, ncol=100)
mpl$setFilter(f)

max_ep = 300

#trs = seq(0.01, 0.1, length.out=max_ep)

start_f = mpl$getFilter()

err = function(r, x) {
    sum((r-x[1:length(r)])^2)
}
norm = function(x) {
    x/sqrt(sum(x^2))
}

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


for(ep in 1:max_ep) {
    inp = v[["values"]][[1]][1:1000]
    ret = mpl$run(inp)
    spikes = ret$spikes
    
    f = mpl$getFilter()
    restored = mpl$restore(spikes)
    cat("restored 0:", err(restored, inp), "\n")
    for(sp in spikes) {
        sp_t = (sp$t+1)
        t_slice = sp_t:(sp_t+ncol(f)-1)
        delta = inp[t_slice] - restored[t_slice]
        
#                 plot(t_slice, inp[sp_t:(sp_t+ncol(f)-1)], type="l", main=sp$fi)
#                 lines(t_slice, restored[sp_t:(sp_t+ncol(f)-1)],  col="red")
#                 lines(t_slice, delta,  col="blue")
        #number <- scan(n=1)
        
        f[sp$fi+1,] <- f[sp$fi+1,] +  sp$s * delta
        
#                 f = t(sapply(1:nrow(f), function(i) norm(f[i,])))
#                 mpl$setFilter(f)
#                 restored2 = mpl$restore(spikes)
#                 delta2 =   inp[t_slice] - restored2[t_slice]
#                 
#                 plot(t_slice, inp[t_slice], type="l", main=sp$fi)
#                 lines(t_slice, restored2[t_slice], col="red")
#                 lines(t_slice, delta2, col="blue")
        #number <- scan(n=1)
        
    }
    
    f = t(sapply(1:nrow(f), function(i) norm(f[i,])))
    mpl$setFilter(f)
    restored_new = mpl$restore(spikes)
    cat("restored 1:", err(restored_new, inp), "\n")
}

end_f = mpl$getFilter()
#gr_pl(end_f)

x_d = mpl$restore(spikes)[1:500]
x = inp[1:500]
plot(x, ylim=c(min(x_d,x), max(x,x_d)), type="l")
lines(x_d,col="red")


