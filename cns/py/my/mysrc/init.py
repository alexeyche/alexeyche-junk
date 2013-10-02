# This file contains all the parameters.
# It is called by main.py

print '********'
print '* Init *'
print '********'

import os
isunix = lambda: os.name == 'posix'
import matplotlib   
import re

#if isunix():
#    matplotlib.use('Agg') # no graphic link with Unix cluster for me

import brian_no_units
from brian import * 
from scipy.io import *
from scipy import weave
from time import time
from time import localtime
from customrefractoriness import *
import pickle
import glob
import numpy

set_global_preferences(useweave=True) # to compile C code

globalStartTime=time()*second


#*************************
# COMPUTATION PARAMETERS *
#*************************

get_default_clock().set_dt(.1*ms) # set time step

# random state
randState = 0 # use this to specify it

seed(randState)

imposedEnd = Inf*second # imposed end time
N = 100 # number of presynaptic neurons
nG = 1 # number of gmax values
nR = 1 # number of ratio LTD/LTP
M = nG*nR # number of postsynaptic neurons, numbered like that [ (r_0,g_0)...(r_0,g_nG),(r_1,g_0)...(r_1,g_nG),...,(r_nR,g_0)...(r_nR,g_nG)]

useSavedWeight = False # load previously dumped weights in ../data/weight.###.mat (if False, or if file not found then random initial weights will be used)
timeOffset = 0*second # simulation starts at t=timeOffset
jitter = 0*second #0e-3*second # add jitter to the input spike trains

graph = True # graph output
monitorInput = True # monitor input spikes
monitorOutput = True # monitor output spikes
monitorPot = True # monitor potential in output layer
monitorCurrent = True # monitor potential in output layer
monitorRate = True # monitor rates in output layer
isMonitoring = False # flag saying if currently monitoring
monitorTime = 0*second
analyzePeriod = 40 # periodically dump weights

   
#**************************
# NEURON MODEL PARAMETERS *
#**************************

# Types of output neurons
conductanceOutput = False # conductance-base output neurons (as opposed to LIF)
poissonOutput = True # stochastic (Poisson) output neurons (as opposed to deterministic). Note that their differential equations are the same as the LIF, but firing is stochastic.

#neurons (Dayan&Abbott 2001)
refractoryPeriod = 1*ms
R=10*Mohm
if poissonOutput:
    vt=400 # not used (just for graph scaling)
    vr=-450 # not used
    #El=-200 # resting 
    El = 0
else:
    vt=-54*mV # threshold
    vr=-60*mV # reset
    El=-70*mV # resting
    Ee=0*mV # for excitatory conductance
taum = 10*ms # membrane time constant
taue=taum/4 # synapse time constant

sigma = 0*0.015*(vt-vr) # white Gaussian noise. Applies to input and output neurons

# array of max conductance values
unitaryEffect = taue/(taum-taue)*((taum/taue)**(-taue/(taum-taue))-(taum/taue)**(-taum/(taum-taue))) # this corresponds to the maximum of the kernel (exp(-t/taum) - exp(-t/taue)) taue / (taum-taue)
if poissonOutput:
    gmax=0.1125/taue*exp(2*log(1.05)*(array(nR*range(nG))-nG/2)) # taue is in factor because the kernel in the paper is normalized
else: # LIF or gIF    
    gmax = 0.028*(vt-El)/unitaryEffect*exp(2*log(1.05)*(array(nR*range(nG))-nG/2)) # 1/0.028 is the number of synchronous input spikes arriving through maximally potentiated synapses needed to reach the threshold from the resting state
    gmax=[6.]

#if conductanceOutput:
#    gmax /= (Ee-vt)

print 'gmax=' + str(gmax)

#********************
# WEIGHT PARAMETERS *
#********************
# initial synaptic weight are randomly picked (uniformly) between those two bounds
if poissonOutput: #
    initialWeight_min = 0 #0*7000.0/N # 0
    initialWeight_max = 3 #2*7000.0/N
else:
    initialWeight_min = 0*volt
#    initialWeight_max = 2000.0/N*35e-5*volt
    initialWeight_max = 45

burstingCriterion = .8 # unplug neurons whose mean normalized synaptic weight is above this value. This allows to save memory by not computing neurons whose normalized synaptic weights all go to 1.

#******************
# STDP PARAMETERS *
#******************    
nearestSpike = False # Implement "nearest spike" mode for STDP
tau_post=34.0*ms #(source: Bi & Poo 2001)
tau_pre=17.0*ms #(source: Bi & Poo 2001)
eta = 1e-3# learning rate
a_pre= 1.0 * eta # LTP magnitude
w_out = - 1.0 * eta # homeostatic rate-based terms (LTD)

# homeostatic rate-based terms (LTP):
w_in=zeros(M) # array of w_in/w_out ratios
for i in range(nR):
    w_in[i*nG:(i+1)*nG] = -w_out*.5*exp(0*(i-nR/2)*.5*log(2))
print 'w_in:', w_in
print 'w_out:', w_out

a_post=zeros(M) # array of LTD/LTP ratios
for i in range(nR):
    a_post[i*nG:(i+1)*nG] = -a_pre*1.05**-4*exp((i-nR/2)*2*log(1.05)) #

print 'a_pre:', a_pre
print 'a_post:', a_post
print 'normalized a_post/a_pre ratios' + str(a_post/a_pre)
    

mu = 0 # to interpolate between additive and multiplicative STDP (see Gutig et al. 2003)

#***********
# FUNCIONS *
#***********

def printtime(mess):
    t = localtime()
    print  '%02d' % t[3] + ':' + '%02d' % t[4] + ' ' + mess

# Called whenever output neurons fire. Resets the potential and trigger STDP updates.
# Includes C code, will be compiled the first time it is called
# Param:
# P: NeuronGroup (output neurons)
# spikes: list of the indexes of the neuron that fire.
def neurons_reset(P,spikes):
    if size(spikes):
        if not poissonOutput:
            P.v_[spikes]=vr # reset pot
        if nearestSpike:
            nspikes = size(spikes)
            A_pre = mirror.A_pre_
            code = '''
            for(int si=0;si<nspikes;si++)
            {
                int i = spikes(si);
                for(int j=0;j<N;j++)
                {
                    double wnew;
                    wnew = _synW(j,i)+_gmax(i)*w_out;
                    if(wnew<0) wnew = 0.0;
                    if(!_alreadyPotentiated(j,i))
                    {
                        if(mu==0) { /* additive. requires hard bound */
                            wnew += _gmax(i)*A_pre(j);
                            if(wnew>_gmax(i)) wnew = _gmax(i);
                        }
                        else { /* soft bound */
                            wnew += _gmax(i)*A_pre(j)*exp(mu*log(1-wnew/_gmax(i)));
						}
                        _alreadyPotentiated(j,i) = true;
                    }
                    _synW(j,i) = wnew;
                }
            }
            '''
            weave.inline(code,
                        ['spikes', 'nspikes', 'N', '_alreadyPotentiated', '_synW', '_gmax', 'A_pre', 'mu','w_out'],
                        compiler='gcc',
                        type_converters=weave.converters.blitz,
                        extra_compile_args=['-O3'])
            _alreadyDepressed[:,spikes] = False
            P.A_post_[spikes]=a_post[spikes] # reset A_post (~start a timer for future LTD)

        else: # all spikes
            nspikes = size(spikes)
            A_pre = mirror.A_pre_
            code = '''
            for(int si=0;si<nspikes;si++)
            {
                int i = spikes(si);
                for(int j=0;j<N;j++)
                {
                        double wnew;
                        if(mu==0){ /* additive. requires hard bound */
                            wnew = _synW(j,i)+_gmax(i)*(w_out+A_pre(j));
                            if(wnew>_gmax(i)) wnew = _gmax(i);
                            if(wnew<0) wnew = 0.0;
                        }
                        else { /* soft bound */
                            wnew = _synW(j,i)+_gmax(i)*(w_out+A_pre(j)*exp(mu*log(1-_synW(j,i)/_gmax(i))));
                            if(wnew>_gmax(i)) wnew = _gmax(i);
                            if(wnew<0) wnew = 0.0;
						}
                        _synW(j,i) = wnew;
                }
            }
            '''
            weave.inline(code,
                        ['spikes', 'nspikes', 'N', '_synW', '_gmax', 'A_pre', 'mu','w_out'],
                        compiler='gcc',
                        type_converters=weave.converters.blitz,
                        extra_compile_args=['-O3'])
            P.A_post_[spikes]+=a_post[spikes] # reset A_post (~start a timer for future LTD)

# Called whenever input neurons fire. Resets the potentials and trigger STDP updates.
# Note that mirror is a fake group that mirrors input neuron, only used for implementation issues.
# Includes C code, will be compiled the first time it is called
# Param:
# P: NeuronGroup (input neurons)
# spikes: list of the indexes of the neuron that fire.
def mirror_reset(P,spikes):
    if size(spikes):
        P.v_[spikes] = 0
        if nearestSpike:
            nspikes = size(spikes)
            A_post = neurons.A_post_
            code = '''
            for(int si=0;si<nspikes;si++)
            {
                int i = spikes(si);
                for(int j=0;j<M;j++)
                {
                    double wnew;
                    wnew = _synW(i,j)+_gmax(j)*w_in(j);
                    if(wnew>_gmax(j)) wnew = _gmax(j);
                    if(!_alreadyDepressed(i,j))
                    {
                        if(mu==0) { /* additive. requires hard bound */
                            wnew += _gmax(j)*A_post(j); 
                            if(wnew<0.0) wnew = 0.0;
                        }
                        else { /* soft bound */
                            wnew += _gmax(j)*A_post(j)*exp(mu*log(wnew/_gmax(j)));
						}
                        _alreadyDepressed(i,j) = true;
                    }
                    _synW(i,j) = wnew;
                }
            }
            '''
            weave.inline(code,
                        ['spikes', 'nspikes', 'M', '_alreadyDepressed', '_synW', '_gmax', 'A_post', 'mu','w_in'],
                        compiler='gcc',
                        type_converters=weave.converters.blitz,
                        extra_compile_args=['-O3'])
            _alreadyPotentiated[spikes,:]=False
            P.A_pre_[spikes]=a_pre  # reset A_pre (~start a timer for future LTP)
            
        else: # all spikes
            nspikes = size(spikes)
            A_post = neurons.A_post_
            code = '''
            for(int si=0;si<nspikes;si++)
            {
                int i = spikes(si);
                for(int j=0;j<M;j++)
                {
                        double wnew;
                        if(mu==0) { /* additive. requires hard bound*/
                            wnew = _synW(i,j)+_gmax(j)*(w_in(j)+A_post(j));
                            if(wnew>_gmax(j)) wnew = _gmax(j);
                            if(wnew<0.0) wnew = 0.0;
                        }
                        else { /* soft bound */
                            wnew = _synW(i,j)+_gmax(j)*(w_in(j)+A_post(j)*exp(mu*log(_synW(i,j)/_gmax(j))));
                            if(wnew>_gmax(j)) wnew = _gmax(j);
                            if(wnew<0.0) wnew = 0.0;
						}
                        _synW(i,j) = wnew;
                }
            }
            '''
            weave.inline(code,
                        ['spikes', 'nspikes', 'M', '_synW', '_gmax', 'A_post', 'mu','w_in'],
                        compiler='gcc',
                        type_converters=weave.converters.blitz,
                        extra_compile_args=['-O3'])
            P.A_pre_[spikes]+=a_pre  # reset A_pre (~start a timer for future LTP)
