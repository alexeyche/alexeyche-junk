

require(Rsnn)

N = 100

mean_rate = 10 # Hz

correllated_neurons = 5
correlation = 0.3

len = 1000 # ms

net = vector("list",N)
for(ni in 1:N) {
    spikes_bin = rpois(len, mean_rate/1000.0)  # ms precision
    net[[ni]] = which(spikes_bin == 1)
}

prast(net)