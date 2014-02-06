
#data cfg
samples_from_dataset = 10

# epsp
e0 <- 1.3 # mV
ts <- 1 # ms
tm <- 10 # ms

refr_mode = 'high' # 'middle', 'low'

# stochastic treshold
alpha <- 0.71
beta <- 0.72
tr <- -50 # mV

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
start_w.M.mean = 10
start_w.M.sd = 0.5
start_w.N.mean = 10
start_w.N.sd = 0.5

# learning
learn_window_size = duration/2
llh_depr_mode = 'no' # 75 spikes per window

lr = 1

epochs = 100
weights_norm_type = 'mult_local'
