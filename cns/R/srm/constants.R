
seed_num = 1234

#data cfg
samples_from_dataset = 10

# epsp
e0 <- 1 # mV
ts <- 1 # ms
tm <- 20 # ms

# epsp suppr
ta = 50

refr_mode = 'high' # 'middle', 'low'

# stochastic treshold
alpha <- 0.70
beta <- 0.70
tr <- -50 # mV

gain_factor = 12.5
pr = 1

# srm
u_rest <- -70 # mV


# input spike train
duration = 500
dt = 0.5
sim_dim = 1000

# net cfg
N = 10
M = 50
net_edge_prob = 0.75
inhib_frac = 0.0
net_neurons_for_input = N
afferent_per_neuron = M

# start weights
start_w.M.mean = 0.25
start_w.M.sd = 0.01
start_w.N.mean = 0.25
start_w.N.sd = 0.01

# learning
learn_window_size = duration/2
llh_depr_mode = 'no' # 75 spikes per window

tc = 100
lr = 11
epochs = 100
weights_norm_type = 'no'
ws = 0.2
target_rate = 5/sim_dim 
target_rate_factor = 0.1
weight_decay_factor = 0.026

mean_time =  1000 #60*sim_dim

added_lrate = 1
