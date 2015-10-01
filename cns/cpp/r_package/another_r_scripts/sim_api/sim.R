
require(Rdnn)

setwd(path.expand("~/dnn/r_scripts"))
source("gen_poisson.R")

Ninput=100
Noutput=100
rate=10
len=1000
jobs = 1
start_weight = 0.15

s = RSim$new()
const = s$getConst()

const$setElement(
    "LeakyIntegrateAndFire", 
    list(tau_m=5.0, rest_pot=0.0, tau_ref=2.0, noise=0.2)
)

const$setElement("Stochastic", list(prob=0.5))

const$addLayer(
    list(neuron="SpikeSequenceNeuron", size=Ninput)
)
const$addLayer(
    list(size=Noutput, neuron="LeakyIntegrateAndFire", act_function="Determ")
)
const$addConnection(0, 1, list(
    type="Stochastic", start_weight=start_weight, synapse="StaticSynapse"
))

s$build()



spikes = gen_poisson(Ninput, rate, len)
s$setInputSpikes(spikes, "SpikeSequenceNeuron")
proto.write("~/dnn/spikes/test_spikes.pb", list(values=spikes), "SpikesList")

s$run(jobs)
net = s$getSpikes()
prast(net)
