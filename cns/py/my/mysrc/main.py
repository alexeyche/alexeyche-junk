# Main file.

execfile('init.py') # sets the parameters

#****************************************************
printtime('************************')
printtime('* Computing (rand=' + '%03d' % randState + ') *')
printtime('************************')

# for memory issues the simulation is divided in multiple periods
# count is the current period number
count = 0

    
_gmax = asarray(gmax) # faster implementation (does not verify dimensions)

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
    raise Exception("Not implemented yet")
    #neurons=NeuronGroup(M,model=eqs_neurons,threshold=vt,reset=neurons_cr)    

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


