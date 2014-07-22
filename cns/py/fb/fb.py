


from brian import *

N = 250

defaultclock.dt = .01 * ms

tau = 1 * ms # Decay time constant of filters = 2*tau
freq = linspace(100 * Hz, 2000 * Hz, N) # characteristic frequencies


eqs = '''
dv/dt=(-a*w-v+I)/tau : Hz
dw/dt=(v-w)/tau : Hz # e.g. linearized potassium channel with conductance a
a : 1
I = gain*(sin(4*pi*f_stimulus*t)+sin(6*pi*f_stimulus*t)) : Hz
'''


inp = SpikeGeneratorGroup(N, spikeList)

neur = NeuronGroup(N, model=eqs, threshold=PoissonThreshold())

con = IdentityConnection(inp, neurons)
