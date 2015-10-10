require(Rdnn)
require(rjson)
pe = path.expand

convSpikeToListOfNeurons = function(spikes) {
    net = blank_net(max(unique(spikes$fi))+1)
    for(i in 1:length(spikes$fi)) {
        net[[ spikes$fi[i]+1 ]] = c(net[[ spikes$fi[i]+1 ]], spikes$t[i])
    }
    return(net)
}

input = RProto$new("/home/alexeyche/dnn/ts/riken_14chan_3LRR.pb")$read()
v = input[["values"]]
setwd(pe("~/dnn/spikes/from_mpl"))

conf = fromJSON(file="mpl.json")[[1]]
mpl = RMatchingPursuit$new(conf)

filter = RProto$new("filter.pb")$read()[[1]]
mpl$setFilter(filter)

net = NULL
for(d in 1:length(v)) {
    ret = mpl$run(v[[d]])
    spikes = ret$spikes  
    net = c(net, convSpikeToListOfNeurons(spikes))    
}

sl_out = list()
sl_out$values = net
sl_out$ts_info = input$ts_info
sl_out$values = net

RProto$new("spikes_list.pb")$write(sl_out, "SpikesList")