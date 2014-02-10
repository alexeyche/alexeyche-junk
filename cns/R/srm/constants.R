
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
duration = 300
dt = 0.5

# net cfg
N = 50
M = 50
net_edge_prob = 0.1
inhib_frac = 0.25
net_neurons_for_input = ceiling(N/2)
afferent_per_neuron = ceiling(M/5)

# start weights
start_w.M.mean = 11
start_w.M.sd = 1
start_w.N.mean = 11
start_w.N.sd = 1

# learning
learn_window_size = duration/2
llh_depr_mode = 'no' # 75 spikes per window

lr = 0.5
rew_ampl = 1

epochs = 50
weights_norm_type = 'no'
ws = 0.2

sim_dim = 1000
