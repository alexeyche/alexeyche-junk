#!/usr/bin/PScript

from brian import *
import numpy
import random
from time import *
import multiprocessing
import itertools
import gc

#draw n normally distributed random numbers in the range [start,stop] 
def draw_normals(n,start,stop):
    mu,sigma,numbers = start+(stop-start)/2, (stop-start)/6, zeros(n)
    for i in range(n):
        s = -1
        while (s<start) or (s>stop) :        
            s = numpy.random.normal(mu,sigma,1)
        numbers[i]=s
    return numbers


eqs = Equations(''' dv/dt = (0.04/ms/mV)*v**2 + (5/ms) * v + 140*mV/ms - u + I_syn/ms + I_in/ms : volt
                    du/dt = a*((b*v) - u)                                             : volt/second
                    dx/dt = -x/(1*ms)                                                 : 1
                    I_in  = ceil(x)*(x>(1/exp(1)))*amplitude                          : volt 
                    dI_syn/dt = - I_syn/tau                                           : volt
                    a                                                                 : 1/second
                    b                                                                 : 1/second
                    c                                                                 : volt
                    d                                                                 : volt/second
                    amplitude                                                         : volt
                    tau                                                               : msecond
                    ''')
#reset specification of the Izhikevich model
reset = '''
v = c
u += d
'''
n= 100
R= 0.5
#2nd: Define the Population of Neurons P
P = NeuronGroup(n, model=eqs, threshold=30*mvolt, reset=reset)
#3rd: Define subgroups of the neurons (regular spiking/fast spiking)
Pe   = P.subgroup(int(n*R))
Pi   = P.subgroup(n-int(n*R))
#4th: initialize starting neuronal parameters for the simulation
Pe.a = 0.02/msecond
Pe.b = 0.2/msecond
Pe.c = (15*draw_normals(int(n*R),float(0),1) - 65)            * mvolt
Pe.d = (-6*draw_normals(int(n*R),float(0),1) + 8)             * mvolt/msecond
Pe.tau = draw_normals(int(n*R),float(3),5)                    * msecond
Pi.a = (0.08*draw_normals(n-int(n*R),float(0),1) + 0.02)      * 1/msecond
Pi.b = (-0.05*draw_normals(n-int(n*R),float(0),1) + 0.25)     * 1/msecond
Pi.c = -65                                                    * mvolt
Pi.d = 2                                                      * mvolt/msecond
Pi.tau = draw_normals(n-int(n*R),float(15),20)                * msecond    
P.x, P.amplitude = 0,draw_normals(n,0,8)                      * mvolt

#5th: initialization of resting membrane potential    
Pe.v = Pe.c
Pi.v = Pi.c

#6th: initialization of u
Pe.u = Pe.b*Pe.c
Pi.u = Pi.b*Pi.c

"""Input to the whole network by synaptic noise"""
G = PoissonGroup(n, rates = 330 * hertz)
GC = IdentityConnection(G, P, 'x', weight=1)
   
"""Monitors, plotting, run of the simulation"""
M_spikes = SpikeMonitor(P)  
run(1 * second,report='text')
raster_plot(M_spikes)    
show()

