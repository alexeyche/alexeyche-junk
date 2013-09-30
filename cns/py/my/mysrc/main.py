# Main file.

execfile('init.py') # sets the parameters

#****************************************************
printtime('************************')
printtime('* Computing (rand=' + '%03d' % randState + ') *')
printtime('************************')

# for memory issues the simulation is divided in multiple periods
# count is the current period number
count = 0

if nearestSpike: # flags useful for nearest spike mode
    alreadyDepressed = zeros([N,M],dtype=int)
    alreadyPotentiated = zeros([N,M],dtype=int)


    
_gmax = asarray(gmax) # faster implementation (does not verify dimensions)

if nearestSpike: # flags useful for nearest spike mode
    _alreadyPotentiated = asarray(alreadyPotentiated)
    _alreadyDepressed = asarray(alreadyDepressed)

# mirrors
mirror_eqs=''' 
v:1
dA_pre/dt=-A_pre/tau_pre : 1
''' 
mirror = NeuronGroup(N, model=mirror_eqs, threshold=0.5, reset=mirror_reset)

#STDP neuron(s)
if conductanceOutput:
    eqs_neurons='''
    dv/dt=(ge*(Ee-v)+El-v)/taum + sigma*xi/taum**.5 : volt
    dge/dt=-ge/taue : 1
    dA_post/dt=-A_post/tau_post : 1
    '''        
else:
    eqs_neurons='''
    dv/dt=(ge+El-v)/taum + sigma*xi/taum**.5 : volt
    dge/dt=-ge/taue : volt
    dA_post/dt=-A_post/tau_post : 1
    '''

neurons_cr = CustomRefractoriness(neurons_reset,period=refractoryPeriod,state='v')

if poissonOutput: # stochastic spike generation
    neurons=NeuronGroup(M,model=eqs_neurons,threshold=PoissonThreshold(), reset=neurons_cr)
else: # deterministic spike generation
    #raise Exception("Not implemented yet")
    neurons=NeuronGroup(M,model=eqs_neurons,threshold=vt, reset=neurons_cr)    

#connections
synapses=Connection(mirror,neurons,'ge',structure='dense')
synapses_h=Connection(mirror_h, neurons,'ge',structure='dense')

seed(randState)

if useSavedWeight and os.path.exists(os.path.join('..','data','weight.'+'%03d' % (randState)+'.mat')):
    print 'Loading previously dumped weight'    
    tmp=loadmat(os.path.join('..','data','weight.'+'%03d' % (randState)+'.mat'),squeeze_me=False)
    tmp=tmp['weight']     
    initialWeight = zeros([N,M])
    for j in range(M):
        initialWeight[:,j] = tmp[:,j]*gmax[j]
    del tmp
else: # start from random synaptic weights
    initialWeight = zeros([N,M])
    for i in range(N):
        initialWeight[i,:] = 0.2*volt
        #initialWeight[i,:] = initialWeight_min + rand(M)*(initialWeight_max-initialWeight_min)
    if initialWeight.max() > min(gmax):
        print '***********************************************************'
        print '* WARNING: Initial weight > gmax. This should not happen. *'
        print '***********************************************************'
synapses.connect(mirror,neurons,initialWeight)
synapses.compress() 
_synW = asarray(synapses.W)

# affect initial values
neurons.v_ = 0#vr+rand(1)*ones(len(neurons))*(vt-vr)

neurons.A_post_=0*volt
neurons.ge_=0*volt
mirror.A_pre_=0*volt
     
startTime = timeOffset;

            
# spikes are read from files spikeList.###.###.mat (first number: random seed, second number: file number) 
    
printtime('Starting (use saved spike list)')

# patterns
from genPattern import spikeAvalanche, spikeAvalancheBack

aval = []
aval.append(spikeAvalanche(nAffer = N, dt = 0.005, T=100))
#aval.append(spikeAvalancheBack(nAffer = N, dt = 0.05, T=5))
#aval.append(spikeAvalanche(nAffer = N, dt = 0.05, T=5))
#aval.append(spikeAvalanche(nAffer = N, dt = 0.05, T = 10))

t_aval = 0
for i in range(0, len(aval)):
    aval[i][:,1] += t_aval    
    t_aval = aval[i][-1,1]

#aval = [numpy.asarray([[0,3],[1,5]])]

for sl in aval:
    localStartTime = time()*second
   
    sl=sl[sl[:,1]>=timeOffset,:] # otherwise Brian sends the old spikes
    if jitter>0:
        sl[:,1] += jitter*randn(size(sl,0))
        sl=sl[sl[:,1].argsort(),]
    print str(size(sl,0)) + ' spikes read (in ' + str(time()*second-localStartTime) + ')'


    endTime = sl[-1][1]       
    input = SpikeGeneratorGroup(N, sl) # special Brian NeuronGroup that fire at specified dates
    if endTime>=monitorTime and not isMonitoring:
   
        print '********************'            
        print '* Start monitoring *'            
        print '********************'            
        isMonitoring = True
        if monitorInput:
            inputSpike = SpikeMonitor(input,True)
        if monitorOutput:
            outputSpike = SpikeMonitor(neurons,True)
        if monitorPot:
            if poissonOutput:
                pot = StateMonitor(neurons,'v',record=True,timestep=10)
            else:
                pot = StateMonitor(neurons,'v',record=True,timestep=10)
        if monitorCurrent:
            current = StateMonitor(neurons,'ge',record=True,timestep=1)
        if monitorRate:
            rate = []
            for i in range(M):
                rate.append(PopulationRateMonitor(neurons[i],bin=2000*ms))
     # imposed end time
    endTime = min(imposedEnd,endTime)
    C_input_mirror = IdentityConnection(input, mirror)


    # run
    print 'Running from t=' + str(startTime) + ' to t=' + str(endTime)
    defaultclock.reinit(startTime) # make sure end time is exactly the one we want, to avoid drifting
    run(endTime-startTime) # run Brian simulator until endTime
    startTime = endTime
    print _synW
#    finalWeight = zeros([N,M])
#    for i in range(N):
#        for j in range(M):
#            finalWeight[i,j] = _synW[i,j]/gmax[j]

               
print 'Total computation time: ' + str(time()*second-globalStartTime)

figure(1)
if monitorPot:
    subplot(311)
    plot(pot[0])

if monitorOutput:
    subplot(312)
    raster_plot(outputSpike)

if monitorCurrent:
    subplot(313)
    plot(current[0])



show()
