# Compute mutual info between the presence of the stimulus and the postynaptic responses
# Results are plotted, and dumped in data.mat

from scipy.stats import norm

printtime('**************************')
printtime('* Mutual Info (rand=' + '%03d' % randState + ') *')
printtime('**************************')

# note: patternPeriod should be loaded

eval = array([ max(0,round(endTime)-500.0), round(endTime) ]) # period of evaluation


# load pattern periods     
if os.path.exists(os.path.join('..','data','patternPeriod.'+'%03d' % (randState)+'.mat')):
    patternPeriod=loadmat(os.path.join('..','data','patternPeriod.'+'%03d' % (randState)+'.mat'))
    patternPeriod=patternPeriod['patternPeriod']
    patternPeriod[:,1]=patternPeriod[:,0]+25e-3
for i in range(1):
    
    #dt = min(100*second,endTime) # period of evaluation
    
    
    
    
    #idx = 0; # cursor for patternPeriod array
    #
    ## init arrays
    #nH = zeros(M)
    #nFA = zeros(M)
    
    #if a>0: #oscillatory mode
    #    # gather data
    #    for i in range(outputSpike.nspikes):
    #        if outputSpike.spikes[i][1]<endTime-dt:
    #            continue
    #        while idx<len(patternPeriod) and  patternPeriod[idx][1] + .5/oscilFreq  < outputSpike.spikes[i][1]:
    #            idx+=1
    #        if idx<len(patternPeriod) and patternPeriod[idx][0] + .5/oscilFreq < outputSpike.spikes[i][1] and patternPeriod[idx][1]-patternPeriod[idx][0]>.5/oscilFreq:
    #            nH[outputSpike.spikes[i][0]] += 1
    #        else:
    #            nFA[outputSpike.spikes[i][0]] += 1
    #
    #else:
    #    if useReset: # reset mode
            
#    evalPeriod = 1/oscilFreq# time bins
    evalPeriod = 25e-3# time bins
    if poissonOutput:
        nSpikeCriterion = 1 # spike number threshold
    else:
        nSpikeCriterion = 1 # spike number threshold
        
    latencyRange = [-20e-3] #[ -50e-3 ] #1e-3*arange(0,0,25) # causes a 1/2 latency shift between stimulus times and response times
    maxI = zeros(len(latencyRange))
    
    for l in range(len(latencyRange)):
        latency = latencyRange[l]
        nSpikeCriterion = l+1 # tmp
        
        idxP = 0 # cursor for pattern periods
        idxS = 0 # cursor for postsynaptic spikes
        
        nHit = zeros(M)
        nMissed = zeros(M) 
        nFA = zeros(M) # false alarm
        nCR = zeros(M) # correct rejection
        
            
        # move to pertinent pattern periods
        while idxP<len(patternPeriod) and  patternPeriod[idxP][1] < eval[0]:
            idxP+=1
        # move to pertinent spikes
        while idxS<outputSpike.nspikes and  outputSpike.spikes[idxS][1]-.5*latency< eval[0]:
            idxS+=1
        
        for i in range(int((eval[1]-eval[0])/evalPeriod)):
            t = eval[0]+i*evalPeriod
            # determine if evalPeriod is mostly a stimulus or a distractor
            patternTime = 0
            while idxP<len(patternPeriod) and  patternPeriod[idxP][0] < t+evalPeriod:
                patternTime += min(t+evalPeriod,patternPeriod[idxP][1]) - max(t,patternPeriod[idxP][0])
                if patternPeriod[idxP][1] < t+evalPeriod:
                    idxP+=1
                else:
                    break
        #            print patternTime
        
            # count spikes
            spikeCount = zeros(M)
            while idxS<outputSpike.nspikes and  outputSpike.spikes[idxS][1]-.5*latency< t+evalPeriod:            
                spikeCount[outputSpike.spikes[idxS][0]] += 1
                idxS+=1
            
            if patternTime>.5*evalPeriod: # stimulus bin
                nHit = nHit + (spikeCount>=nSpikeCriterion)
                nMissed = nMissed + (spikeCount<nSpikeCriterion)
            else: # distractor bin
                nFA = nFA + (spikeCount>=nSpikeCriterion)
                nCR = nCR + (spikeCount<nSpikeCriterion)
        
             
        I = zeros(M)        
        dprime = zeros(M)
        card = int((eval[1]-eval[0])/evalPeriod)
        for m in range(M):
            if nHit[m]>0:
                I[m] += nHit[m]      * log( card * nHit[m]/(nFA[m]+nHit[m])/(nMissed[m]+nHit[m]) ) 
            if nFA[m]>0:
                I[m] += nFA[m]       * log( card * nFA[m]/(nFA[m]+nCR[m])/(nFA[m]+nHit[m]) )
            if nMissed[m]>0:
                I[m] += nMissed[m]   * log( card * nMissed[m]/(nMissed[m]+nCR[m])/(nMissed[m]+nHit[m]) )
            if nCR[m]>0:
                I[m] += nCR[m]       * log( card * nCR[m]/(nMissed[m]+nCR[m])/(nFA[m]+nCR[m]) )  
            if nHit[m]>0 and nFA[m]>0 and nMissed[m]>0 and nCR[m]>0:
                dprime[m] = norm.ppf(nHit[m]/(nHit[m]+nMissed[m])) - norm.ppf(nFA[m]/(nFA[m]+nCR[m]))
        I *= 1.0/card * 1/log(2)
        Hs = - 1/log(2) * 1.0/card * ( (nMissed[0]+nHit[0]) * log((nMissed[0]+nHit[0])/card) + (nCR[0]+nFA[0]) * log((nCR[0]+nFA[0])/card) ) 
        
        print 'prop of stim = ' + str((nHit[0]+nMissed[0])/card)
        print 'H(stim) = ' + str(Hs)        
        print 'max(I) = ' + str(max(I))
        print 'mean(I) = ' + str(mean(I))
        print 'std(I) = ' + str(std(I))
        print 'max(I)/H(stim) = ' + str(max(I)/Hs)
        print 'mean(I)/H(stim) = ' + str(mean(I)/Hs)
        print 'max(dprime) = ' + str(max(dprime))

        print ''
    
        maxI[l]=max(I)
    
        if os.path.exists(os.path.join('..','data','data.mat')):
            data=loadmat(os.path.join('..','data','data.mat'),squeeze_me=False)
            data_randState = data['randState']
            data_I=data['I']
            data_dprime=data['dprime']
            data_Hs = data['Hs']
            
            if size(data_randState) == 1:
                data_randState = [data_randState]
                data_I = [data_I]
                data_dprime = [data_dprime]
                data_Hs = [data_Hs]
                
            
            data_randState = concatenate((data_randState,[randState]))
            data_I = concatenate((data_I,[I]))
            data_dprime = concatenate((data_dprime,[dprime]))
            data_Hs = concatenate((data_Hs,[Hs]))
            
        else:
            data_randState = array([randState])
            data_I = array([I])
            data_dprime = array([dprime])
            data_Hs = array([Hs])
        
        data={'randState':data_randState,'I':data_I,'dprime':data_dprime,'Hs':data_Hs}
        savemat('../data/data.mat',data)
        del data
    
    
        if False: #graph:
            figure()
            subplot(2,1,1)
            imshow(reshape(I/Hs,(nR,nG)))
            xlabel('gmax')
            ylabel('LTD/LTP')
            colorbar()
            subplot(2,1,2)
            imshow(reshape(dprime,(nR,nG)))
            xlabel('gmax')
            ylabel('LTD/LTP')
            colorbar()
            
    eval = eval-100 # end of loop on eval periods
         
if len(latencyRange)>1:
    print latencyRange
    print maxI
#    if graph:
#        figure()
#        plot(latencyRange,maxI)


if graph:
    show()
         
#        for t in range(len(reset)):
#            if reset[t]<endTime-dt:
#                continue
#            if reset[t]>=endTime:
#                break
#            if reset[t+1]-reset[t]<evalPeriod: # can not evaluate first reset if a second one follows right afterwards
#                continue            
#            # move on to adequate pattern period
#            while idxP<len(patternPeriod) and  patternPeriod[idxP][1] < reset[t]:
#                idxP+=1
#           
#            # determine if evalPeriod is mostly a stimulus or a distractor
#            idxPT = idxP # temporary pattern cursor
#            patternTime = 0
#            while idxPT<len(patternPeriod) and  patternPeriod[idxPT][0] < reset[t]+evalPeriod:
#                patternTime += min(reset[t]+evalPeriod,patternPeriod[idxPT][1]) - max(reset[t],patternPeriod[idxPT][0])
#                idxPT+=1
#            
#            # see who has detected it
#            hasDetetected = zeros(M)
#            
        
    