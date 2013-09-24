# Main file.

execfile('init.py') # sets the parameters

#****************************************************
printtime('************************')
printtime('* Computing (rand=' + '%03d' % randState + ') *')
printtime('************************')


if nearestSpike: # flags useful for nearest spike mode
    alreadyDepressed = zeros([N,M],dtype=int)
    alreadyPotentiated = zeros([N,M],dtype=int)

# for memory issues the simulation is divided in multiple periods
# count is the current period number
count = 0

    
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
    neurons=NeuronGroup(M,model=eqs_neurons,threshold=PoissonThreshold(),reset=neurons_cr)
else: # deterministic spike generation
    neurons=NeuronGroup(M,model=eqs_neurons,threshold=vt,reset=neurons_cr)    

#connections
synapses=Connection(mirror,neurons,'ge',structure='dense')
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
#            initialWeight[i,:] = initialWeight_min + rand(1)*(initialWeight_max-initialWeight_min)
        initialWeight[i,:] = initialWeight_min + rand(M)*(initialWeight_max-initialWeight_min)
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

# look for spike list files
fileList = listMatFile('../data/',randState)
print str(len(fileList)) + ' spike list files found'


for fl in fileList: # iterate on spile list files

    # read spike list
    localStartTime = time()*second
    print 'Reading '+ fl
    spikeList=loadmat(os.path.join('..','data',fl))
    spikeList=spikeList['sl']
    # spikeList[:,1]+=timeOffset
    spikeList=spikeList[spikeList[:,1]>=timeOffset,:] # otherwise Brian sends the old spikes
    if jitter>0:
        spikeList[:,1] += jitter*randn(size(spikeList,0))
        spikeList=spikeList[spikeList[:,1].argsort(),]
    print str(size(spikeList,0)) + ' spikes read (in ' + str(time()*second-localStartTime) + ')'

    input = SpikeGeneratorGroup(N, spikeList) # special Brian NeuronGroup that fire at specified dates
    endTime = spikeList[-1][1]       
    del spikeList

    # monitors
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
                pot = StateMonitor(neurons,'v',record=True,timestep=1)
        if monitorCurrent:
            current = StateMonitor(neurons,'ge',record=True,timestep=10)
        if monitorRate:
            rate = []
            for i in range(M):
                rate.append(PopulationRateMonitor(neurons[i],bin=2000*ms))

    # imposed end time
    endTime = min(imposedEnd,endTime)

    # connect new spike generator
    C_input_mirror = IdentityConnection(input, mirror)


    # run
    print 'Running from t=' + str(startTime) + ' to t=' + str(endTime)
    defaultclock.reinit(startTime) # make sure end time is exactly the one we want, to avoid drifting
    run(endTime-startTime) # run Brian simulator until endTime

    # periodic graphic plot output
    if floor(endTime/analyzePeriod)!=floor(startTime/analyzePeriod):
        # compute final normalized weight (there's probably a smarter way to do that...)
        finalWeight = zeros([N,M])
        for i in range(N):
            for j in range(M):
                finalWeight[i,j] = _synW[i,j]/gmax[j]
        execfile('saveWeight.py')
#            execfile('analyze.py')

    # start := end
    startTime = endTime

    # explicitly free memory
    del input

    printtime('Period # '+ str(count+1) +': computation time: ' + str(time()*second-localStartTime))
    localStartTime = time()*second
    count += 1

    if endTime>=imposedEnd:
        break

    for j in range(M):
        if mean(_synW[:,j])/gmax[j]>burstingCriterion:
            print 'WARNING: neuron # ' + str(j) + ' is bursting. Disconnecting it.'
            _synW[:,j] = 0*mV
            gmax[j] = 0*mV


print 'Total computation time: ' + str(time()*second-globalStartTime)

# compute final normalized weight (there's probably a smarter way to do that...)
finalWeight = zeros([N,M])
for i in range(N):
    for j in range(M):
        finalWeight[i,j] = _synW[i,j]/gmax[j]

#execfile('pickleAll.py') # pickle all variable (under development...)
if imposedEnd-timeOffset>20: # don't dump short simulations, probably done for display only
   execfile('saveWeight.py') # dump final weights 
    
if monitorPot:
	execfile('savePot.py')

if monitorCurrent:
	execfile('saveCurrent.py')

#execfile('toMatlab.py') # dump variables in a mat file (under development)

if graph: # graphical plot output
    execfile('analyze.py')
    show()

#if imposedEnd>6 and monitorOutput: # mutual info (stimulus, response)
#    execfile('mutualInfo.py')
#    show()
