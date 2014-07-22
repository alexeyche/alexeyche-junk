import random
import math
import numpy as np
#################################################
# Parameters
#################################################

dt = 0.001       # simulation time step  
t_rc = 0.02      # membrane RC time constant
t_ref = 0.002    # refractory period
t_pstc = 0.1     # post-synaptic time constant
N_A = 100         # number of neurons in first population
N_samples = 100  # number of sample points to use when finding decoders
rate_A = 25, 75  # range of maximum firing rates for population A


random.seed(1234)

encoder_A = [1 for i in range(N_A)]

def generate_gain_and_bias(count, intercept_low, intercept_high, rate_low, rate_high):
    gain = []
    bias = []
    for i in range(count):
        # desired intercept (x value for which the neuron starts firing
        intercept = random.uniform(intercept_low, intercept_high)
        # desired maximum rate (firing rate when x is maximum)
        rate = random.uniform(rate_low, rate_high)
        
        # this algorithm is specific to LIF neurons, but should
        #  generate gain and bias values to produce the desired
        #  intercept and rate
        z = 1.0 / (1-math.exp((t_ref-(1.0/rate))/t_rc))
        g = (1 - z)/(intercept - 1.0)
        b = 1 - g*intercept
        gain.append(g)
        bias.append(b)
    return gain,bias

# random gain and bias for the two populations
gain_A, bias_A = generate_gain_and_bias(N_A, -1, 1, rate_A[0], rate_A[1])

# a simple leaky integrate-and-fire model, scaled so that v=0 is resting 
#  voltage and v=1 is the firing threshold
def run_neurons(input,v,ref):
    spikes=[]
    for i in range(len(v)):
        dV = dt * (input[i]-v[i]) / t_rc    # the LIF voltage change equation
        v[i] += dV                   
        if v[i]<0: v[i]=0                   # don't allow voltage to go below 0
        
        if ref[i]>0:                        # if we are in our refractory period
            v[i]=0                          #   keep voltage at zero and
            ref[i]-=dt                      #   decrease the refractory period
        
        if v[i]>1:                          # if we have hit threshold
            spikes.append(True)             #   spike
            v[i] = 0                        #   reset the voltage
            ref[i] = t_ref                  #   and set the refractory period
        else:
            spikes.append(False)
    return spikes
     # call the input function to determine the input value



inputs_test = ( np.genfromtxt("/home/alexeyche/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_1000"), "train" ,)
inputs_train = ( np.genfromtxt("/home/alexeyche/prog/sim/ts/synthetic_control/synthetic_control_TEST_1000"), "test", )

for ds, lab in (inputs_train, inputs_test):
    spikes_all = []
    for xi in xrange(ds.shape[0]):
        input = ds[xi,2:ds.shape[1]]

        v_A = [0.0]*N_A       # voltage for population A
        ref_A = [0.0]*N_A     # refractory period for population A
        input_A = [0.0]*N_A   # input for population A

        spikes = [ list() for i in range(N_A) ]
        t = 0
        for x in input:
            t = t + dt
            # convert the input value into an input for each neuron
            for i in range(N_A):
                input_A[i]=x*encoder_A[i]*gain_A[i]+bias_A[i]
            
            # run population A and determine which neurons spike
            spikes_A=run_neurons(input_A, v_A, ref_A)
            for ni in range(N_A):
                if spikes_A[ni]:
                    spikes[ni].append(t)

        spikes_all.append(spikes)

    mm = max([len(s) for s in [ spikes for spikes in spikes_all ] ])

    sp_m = np.zeros( (N_A*ds.shape[0], mm) )

    for di in range(ds.shape[0]):
        for ni in range(N_A):
            sp_m[di*N_A+ni, :] = np.concatenate( (spikes_all[di][ni], [0]*(mm-len(spikes_all[di][ni])) ))


    np.savetxt("/home/alexeyche/prog/sim/ucr_nengo_spikes/nengo_output_%s.csv" % lab, sp_m, delimiter=",")




