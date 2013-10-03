import brian_no_units
from brian import * 
from scipy.io import *
from scipy import weave
from time import time
from time import localtime
import numpy

N = 100
M = 1

# constants:
eta = 1e-3# learning rate
A_pos = 1 * eta
A_neg = -1 * eta
t_neg=34.0 * 10**-3
t_pos=17.0 * 10**-3
W_len = 0.4 # seconds

vt=-54*mV # threshold
vr=-60*mV # reset
El=-70*mV # resting
Ee=0*mV # for excitatory conductance
taum = 10*ms # membrane time constant
taue=taum/4 # synapse time constant
sigma = 0*0.015*(vt-vr) # white Gaussian noise. Applies to input and output neurons

poissonOutput = False
conductanceOutput = False
#STDP neuron(s)
if conductanceOutput:
    eqs_neurons='''
    dv/dt=(ge*(Ee-v)+El-v)/taum + sigma*xi/taum**.5 : volt
    dge/dt=-ge/taue : 1
    '''        
else:
    eqs_neurons='''
    dv/dt=(ge+El-v)/taum + sigma*xi/taum**.5 : volt
    dge/dt=-ge/taue : volt
    '''


randState = 0

seed(randState)




max_sp_w = 1000 # max spikes in window
post_activity = zeros([M, max_sp_w])
post_activity_n = zeros(M, dtype=numpy.int16)
pre_activity = zeros([N, max_sp_w])
pre_activity_n = zeros(N, dtype=numpy.int16)


def pre_reset(P, spikes):
    if size(spikes):
        if not poissonOutput:
            P.v_[spikes] = vr
        nspikes = size(spikes)    
        t = P.clock.t
        code = '''
        for(int si=0; si<nspikes; si++) {
            int i = spikes(si);
            for(int j=0; j<M; j++) {
                int post_activity_n_cur = post_activity_n(j);
                for(int post_spike=0; post_spike<post_activity_n_cur; post_spike++) {  // from earliest spike to last
                    double u = t-post_activity(j, post_spike);
                    if(u == 0) {
                        continue;
                    }
                    printf("i=%d,u=%1.6f,psi=%d,t=%1.6f\\n",i, u, post_spike, t);
                    if ((u>0)&&(u<W_len/2)) {
                        _synW(i,j) += -A_neg*exp(u/t_neg);
                    } else {
                        post_activity_n(j) -= 1;
                    }
                }
            }
            if(pre_activity_n(i) < max_sp_w) {
                int pi = pre_activity_n(i);
                pre_activity(i, pi) = t;
                pre_activity_n(i) += 1;
                printf("just spike:    i=%d,t=%1.6f,n_act=%d\\n",i, t, pi+1);
            }                
        }
        '''
        weave.inline(code, ['spikes', 'nspikes', 'M', 'pre_activity', 'pre_activity_n', 'post_activity', 'post_activity_n', 'A_neg', 't_neg', '_synW', 'max_sp_w', 'W_len', 't'],
                        compiler='gcc',
                        type_converters=weave.converters.blitz,
                        extra_compile_args=['-O3'])

def post_reset(P, spikes):
    if size(spikes):
        if not poissonOutput:
            P.v_[spikes] = vr
        nspikes = size(spikes)    
        t = P.clock.t
        code = '''
        for(int si=0; si<nspikes; si++) {
            int i = spikes(si);
            for(int j=0; j<N; j++) {
                int pre_activity_n_cur = pre_activity_n(j);
                for(int pre_spike=0; pre_spike<pre_activity_n_cur; pre_spike++) {
                    double u = t-pre_activity(j, pre_spike);
                    if(u == 0) {
                        continue;
                    }
                    printf("                                                     i=%d,u=%1.6f,psi=%d,t=%3.6f\\n",i, u, pre_spike, t);
                    if ((u>0)&&(u<W_len/2)) {
                        _synW(i,j) += A_pos*exp(-u/t_pos);
                    } else {
                        pre_activity_n(j) -= 1;
                    }
                }
            }
            if(post_activity_n(i) < max_sp_w) {
                int pi = post_activity_n(i);
                post_activity(i, pi) = t;
                post_activity_n(i) += 1;
                    printf("                                      just spike:    i=%d,t=%1.6f,n_act=%d\\n",i, t, pi+1);
            }
        }
        '''
        weave.inline(code, ['spikes', 'nspikes', 'N', 'pre_activity', 'pre_activity_n', 'post_activity', 'post_activity_n', 'A_pos', 't_pos', '_synW', 'max_sp_w', 'W_len', 't'],
                        compiler='gcc',
                        type_converters=weave.converters.blitz,
                        extra_compile_args=['-O3'])

