
seed_num = 1234

#data cfg
samples_from_dataset = 1

# epsp
e0 <- 1 # mV
ts <- 1 # ms
tm <- 20 #*10^-3 # ms

# epsp suppr
ta = 50 #*10^-3


# stochastic treshold
alpha <- 1
beta <- 20
tr <- -50 # mV

gain_factor = 12.5
pr = 1

# srm
u_rest <- -70 # mV


# input spike train
duration = 2000
dt = 1
sim_dim = 1000

# net cfg
N = 25
M = 100
net_edge_prob = 0.5
inhib_frac = 1
net_neurons_for_input = N
afferent_per_neuron = M

# start weights
start_w.M.mean = 0.1
start_w.M.sd = 0.001
start_w.N.mean = 0.1
start_w.N.sd = 0.001

# C
tc = 100
mean_p_dur = 60 * sim_dim
ws = 5

target_rate = 5 # Hz
target_rate_factor = 0.1
weight_decay_factor = 0.026

added_lrate = 1
epochs=10

