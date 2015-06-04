
require(Rdnn)
require(rjson)

err = function(r, x) {
    sum((r-x[1:length(r)])^2)
}


input = RProto$new("/home/alexeyche/dnn/ts/riken_14chan_3LRR.pb")$read()
dim = 1

v = input[["values"]][[dim]]

setwd(path.expand("~/cpp/build"))
f = RProto$new("filter.pb")$read()[[1]]

# conf = fromJSON(file="../mpl.json")
# conf = conf$MatchingPursuitConfig
# conf$learn = FALSE

# mpl = RMatchingPursuit$new(conf)
# 
# 
# mpl$setFilter(f[[1]])
# spikes = RProto$new("spikes.pb")$read()
spikes = RProto$new("spikes.pb")$read()


r = RProto$new("restored.pb")$read()[["values"]][[1]]

x = v[1:500]
x_d = r[1:500]
plot(x, ylim=c(min(x_d,x), max(x,x_d)), type="l")
lines(x_d,col="red")
