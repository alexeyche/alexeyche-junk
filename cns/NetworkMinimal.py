"""Import of necessary packages for simulations"""
from brian import *
import numpy
import random
from time import *
import multiprocessing
import itertools
import gc

#defining the piecesize for break up of connections
piecesize=1000

"""@@@@@@@@@Draw numbers from normal distributions in a given range@@@@@@@@@"""
#draw n normally distributed random numbers in the range [start,stop] 
def draw_normals(n,start,stop):
    mu,sigma,numbers = start+(stop-start)/2, (stop-start)/6, zeros(n)
    for i in range(n):
        s = -1
        while (s<start) or (s>stop) :        
            s = numpy.random.normal(mu,sigma,1)
        numbers[i]=s
    return numbers

"""@@@@@@@@@Construct connectivity weight matrix reg. Gritsun 2010@@@@@@@@@"""
#Construct a weight matrix W, where each neuron has maximum K_max exiting connections
#(normally distributed).
def construct_W(K_max, n1, n2, typ):
    #0th: instantiate matrix
    W = zeros(shape=(n1,n2))
       
    #1st: draw K's
    K_vec = map(int,['%.0f' % elem for elem in draw_normals(n1,float(0),K_max)])
    
    #2nd: define connections without self-loops
    for i in range(n1):
        r = range(0,n2)
        r.remove(i)
        c = random.sample(r,K_vec[i])
        for j in c:
            W[i,j]=draw_normals(1,float(0),abs(1)) * mvolt
 
    #4th: return W
    if typ=='ex':
        return W
    else:
        return -W

"""@@@@@@Break up the connections in smaller parts due to compr. prob.@@@@@@"""
#by Dan Goodman
def break_up_connection(source, target, var, D_max, K_max, typ, piecesize=piecesize):
    N = len(source)
    pieces = range(0, N, piecesize)+[N]
    conns = []
    for start, end in zip(pieces[:-1], pieces[1:]): 
        C = DelayConnection(source[start:end], target, var, delay=(0*msecond,D_max * msecond), max_delay=D_max * msecond,
                            column_access=False)
        #construct connectivity-matrix
        W = construct_W(K_max, len(source[start:end]), len(target), typ)
        C.connect(source[start:end],target,W) 
        C.compress()
        #set delays of Ce according to normal distributions
        l = len(W)/2
        unequal1=transpose(nonzero(W[:l][:]))
        unequal2=transpose(nonzero(W[l:][:]))
                
        for i in unequal1:
            C.delay[i[0],i[1]]=draw_normals(1,float(0),D_max) * msecond
        for i in unequal2:
            C.delay[i[0]+l,i[1]]=draw_normals(1,float(0),D_max) * msecond
               
        del W
        gc.collect()
        conns.append(C)
    magic_register(*conns)
    return conns

"""@@@@@@@@@@@@The simulation procedure implementing the network@@@@@@@@@@@@"""
def run_simulation(params):
    reinit_default_clock()      #reset the clock to zero
    clear(True)                 #clear any remaining data
    defaultclock.dt = 1*ms      #set integration timestep to t=1ms
    n, R, K_max, D_max, sim_len, raten, index = params #get the parameters for the procedure
    
    """Define neuron groups"""
    #1st: Create neuronal equations
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
    
    """Define connections"""
    Ce = break_up_connection(Pe, P, 'I_syn', D_max, K_max, 'ex')
    Ci = break_up_connection(Pi, P, 'I_syn', D_max, K_max, 'in')
       
    """Input to the whole network by synaptic noise"""
    G = PoissonGroup(n, rates = raten* hertz)
    GC = IdentityConnection(G, P, 'x', weight=1)
       
    """Monitors, plotting, run of the simulation"""
    M_spikes = SpikeMonitor(P)  
    run(sim_len,report='text')
    raster_plot(M_spikes)    
    show()
        
    gc.collect()
    
"""@@@@@@@Evoking of the simulation procedure for multiple CPU-runs@@@@@@@@@"""
if __name__ == '__main__':
    #Set simulation parameters
    sim_len =  .01  * second   #simulation time 
       
    #Set the network parameters
    n     = 5000            #number of neurons in the network
    #ratio of excitatory to inhibitory neurons
    R     = [0.8] 
    #maximum number of connections of each neuron (normally distr.  btween 0-K_max)
    K_max = [800] 
    #transmission delays in mseconds
    D_max = [5]
    #rates for the Poisson processes     
    raten = 330            
    index = range(size(D_max))
    pool = multiprocessing.Pool() 
    results = pool.map(run_simulation, itertools.izip(itertools.repeat(n), R, K_max, D_max, itertools.repeat(sim_len), itertools.repeat(raten), index)) # launches multiple processes
 
 
