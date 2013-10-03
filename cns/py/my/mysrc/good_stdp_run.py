#!/usr/bin/env python

execfile('good_stdp.py') # sets the parameters

# neurons:

post = NeuronGroup(M, model=eqs_neurons, threshold=vt, reset=post_reset)    
pre = NeuronGroup(N, model="v:1", threshold=0.5, reset=pre_reset)

# synapses

synapses=Connection(pre,post,'ge',structure='dense')
initialWeight = zeros([N,M])
for i in range(N):
    initialWeight[i,:] = 0.05*volt
 
synapses.connect(pre,post,initialWeight)
synapses.compress() 
_synW = asarray(synapses.W)

# affect initial values
post.v_ = vr
post.ge_=0*volt
#imposedEnd = Inf*second
imposedEnd = 5*second
timeOffset=0
startTime = timeOffset


 # patterns
from genPattern import spikeAvalanche, spikeAvalancheBack

aval = []
aval.append(spikeAvalanche(nAffer = N, dt = 0.007, T=10))

t_aval = 0
for i in range(0, len(aval)):
    aval[i][:,1] += t_aval    
    t_aval = aval[i][-1,1]

localStartTime = time()*second

sl = aval[0]

sl=sl[sl[:,1]>=timeOffset,:] # otherwise Brian sends the old spikes
print str(size(sl,0)) + ' spikes read (in ' + str(time()*second-localStartTime) + ')'


endTime = sl[-1][1]       
input = SpikeGeneratorGroup(N, sl) # special Brian NeuronGroup that fire at specified dates
endTime = min(imposedEnd,endTime)

pot = StateMonitor(post,'v',record=True,timestep=1)

C_input_mirror = IdentityConnection(input, pre)

# run
print 'Running from t=' + str(startTime) + ' to t=' + str(endTime)
defaultclock.reinit(startTime) # make sure end time is exactly the one we want, to avoid drifting
run(endTime-startTime) # run Brian simulator until endTime
startTime = endTime
print _synW


plot(pot[0])
show()
