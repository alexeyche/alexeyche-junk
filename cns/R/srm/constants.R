
seed_num = 1234

#data cfg
samples_from_dataset = 5

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
duration = 1000
# sim prop:
dt = 1
sim_dim = 2000

# net cfg
M = 100
N =24
N2 = 24
net_edge_prob = 0.3
net_edge_prob2 = 0.3
inhib_frac = 0.25
inhib_frac2 = 0.25
net_neurons_for_input = N
afferent_per_neuron = M/2

net_neurons_for_input2 = N2
afferent_per_neuron2 = N/4


# start weights
start_W.sd = 0.5
weight_per_neuron = 400
weight_per_neuron2 = 325


# C
tc = 100
mean_p_dur = 60 * sim_dim

target_rate = 5 # Hz
target_rate_factor = 0.1
weight_decay_factor = 0.01 #0.015 #0.026

added_lrate = 1
epochs=100

# postprocess
kernel_sigma = 3
tR = 1
kernel_options = list(T0=0,Tmax=duration, quad=256, sigma=kernel_sigma, tR=tR)

# delays:

syn_delay_rate = 0
axon_delay_rate = 0
delay_dist_gain = 5

