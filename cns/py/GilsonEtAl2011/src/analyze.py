# Graphical plot output

printtime('*************')
printtime('* Analyzing *')
printtime('*************')

outputSelection = range(M)
#outputSelection = arange(6, 52, 13)
#outputSelection = arange(2*13, 3*13, 1)
#outputSelection = [58] # 

# not used anymore (here only for compatibility)
computeOutput=True
monitorInputPot = False 

nbPlot = sum([ monitorInput, monitorInputPot, monitorOutput , monitorCurrent, monitorPot, monitorRate, computeOutput ])
# additional plot w = f(patternValue)
if len(outputSelection)==1 and computeOutput and os.path.exists(os.path.join('..','data','realValuedPattern.'+'%03d' % (randState)+'.mat')):
    nbPlot+=1
## additional plot w = f(index) for 'academic' patterns
#if len(outputSelection)==1 and computeOutput:
#    nbPlot+=1    

useSubplot = False
nCol = 1 # 1
nRow = int(ceil(1.0*nbPlot/nCol))

# end
minTime = max(timeOffset*1000,(endTime-4)*1000)
maxTime = endTime*1000

## beginning
#minTime = 0
##maxTime = 200/oscilFreq*1000
#maxTime = min(endTime,3)*1000

## all
#minTime = 0*second
#maxTime = endTime*1000

#figure()
if useSubplot:
    fig = plt.figure()



idxPlot = 1

if monitorInput:
    # raster plots
    if useSubplot:
        idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
        ax = fig.add_subplot(nRow,nCol,idxPlotTransp,xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)])
    else:
        fig = plt.figure()
        ax = fig.add_subplot(1,1,1,title='Input spikes')
        
    if len(outputSelection)==1: # color code for spikes
        for i in range(inputSpike.nspikes):
            if 1000*inputSpike.spikes[i][1]<minTime-1:
                continue
            if inputSpike.spikes[i][0]>=450 and inputSpike.spikes[i][0]<=550:
                col = finalWeight[inputSpike.spikes[i][0]][outputSelection] * double([1,0,0]) + (1-finalWeight[inputSpike.spikes[i][0]][outputSelection]) * double([0,0,1])
                plot([1000*inputSpike.spikes[i][1]],[inputSpike.spikes[i][0]],'.',markerfacecolor=col,markeredgecolor=col,markersize=3) 
            if 1000*inputSpike.spikes[i][1]>maxTime+1:
                break
    else:
        raster_plot(inputSpike)
        
#    if useReset and maxTime - minTime < 100000:
#        plot([1000*reset,1000*reset],[-1*ones(len(reset)),N*ones(len(reset))],'-r')

#    axis([minTime, maxTime, -1, N])
#    axis([minTime, maxTime, 150, 250])
#    axis([1000*reset[4]-20, 1000*reset[4]+100, 180, 220])
    xlabel('Time (ms)')
    ylabel('Afferent #')
#    title('A')
    text(1.04, 0.5,'A', fontsize=20, horizontalalignment='center',verticalalignment='center',transform = ax.transAxes)
    idxPlot+=1
    if inputSpike.nspikes>0:
        print 'Mean input firing rate = ' + '%.1f' % (inputSpike.nspikes/(endTime-inputSpike.spikes[0][1])/N)
        
    # pattern periods     
    if os.path.exists(os.path.join('..','data','patternPeriod.'+'%03d' % (randState)+'.mat')):
        patternPeriod=loadmat(os.path.join('..','data','patternPeriod.'+'%03d' % (randState)+'.mat'))
        # patternPeriod=(patternPeriod['patternPeriod']+timeOffset)
        patternPeriod=(patternPeriod['patternPeriod'])
        for i in range(size(patternPeriod,0)):
            if 1000*patternPeriod[i,1]<minTime-10000:
                continue
            rect = Rectangle( [1000*patternPeriod[i,0], -.5], 1000*(patternPeriod[i,1]-patternPeriod[i,0]), len(realValuedPattern), facecolor='grey', edgecolor='none')
            ax.add_patch(rect)
            if 1000*patternPeriod[i,0]>maxTime+10000:
                break
            
    print 'Input spikes done'
if monitorInputPot:
    # membrane potential
    subplot(nRow,nCol,idxPlot)
    if useSubplot:
        idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
        ax = fig.add_subplot(nRow,nCol,idxPlotTransp,xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)])
    else:
        fig = plt.figure()
        ax = fig.add_subplot(1,1,1,xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)])
    for j in range(N):
        plot(inputPot.times/(1*msecond),inputPot.values[j,:]/(1*mvolt))
    axis([minTime, maxTime, 1000*(El-.1*(vt-El)), 1000*(vt+.1*(vt-El))])
    xlabel('Time (s)')
    ylabel('Input - Membrane potential (in mV)')
    idxPlot+=1
    print 'Input pot. done'
if monitorCurrent:    
    # current
    if useSubplot:
        idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
        ax = fig.add_subplot(nRow,nCol,idxPlotTransp,xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)])
    else:
        fig = plt.figure()
        ax = fig.add_subplot(1,1,1,xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)])
    for j in outputSelection:
        plot(current.times/(1*msecond),current.values[j,:]/(1*mvolt))
    plot([0, endTime*1000],[1000*(vt-El),1000*(vt-El)],':r',linewidth=2)    
##    axis([0, 300*1000, -1*1000*(vt-El), 5.0*1000*(vt-El)])
    axis([minTime, maxTime, .0*1000*(vt-El), 2.0*1000*(vt-El)])
    xlabel('Time (s)')
    ylabel('R x input current (in mV)')
    idxPlot+=1
    print 'Current done'

if monitorPot:    
    if poissonOutput:
        coef=1
    else:
        coef=1000

    # membrane potential
    if useSubplot:
        idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
        ax = fig.add_subplot(nRow,nCol,idxPlotTransp,xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)])
#        ax = fig.add_subplot(nRow,nCol,idxPlotTransp)
        text(1.04, 0.5,'B', fontsize=20, horizontalalignment='center',verticalalignment='center',transform = ax.transAxes)
    else:
        fig = plt.figure()
        ax = fig.add_subplot(1,1,1,title='Postsynaptic potential',xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)])
    for j in outputSelection:
        plot(pot.times/(1*msecond),coef*pot.values[j,:],label= str(j)+' nW='+str(sum(finalWeight[:,j]>.5,0)))
#        plot(pot.times/(1*msecond),pot.values[j,:]/(1*mvolt))

    # pattern periods     
    if os.path.exists(os.path.join('..','data','patternPeriod.'+'%03d' % (randState)+'.mat')):
        patternPeriod=loadmat(os.path.join('..','data','patternPeriod.'+'%03d' % (randState)+'.mat'))
        # patternPeriod=(patternPeriod['patternPeriod']+timeOffset)
        patternPeriod=(patternPeriod['patternPeriod'])
        for i in range(size(patternPeriod,0)):
            if 1000*patternPeriod[i,1]<=minTime-10000:
                continue
            rect = Rectangle( [1000*patternPeriod[i,0],coef*(El-.1*(vt-El))], 1000*(patternPeriod[i,1]-patternPeriod[i,0]), coef*(1.2*(vt-El)), facecolor='grey', edgecolor='none')
            ax.add_patch(rect)
            if 1000*patternPeriod[i,0]>=maxTime+10000:
                break
    
    xlabel('Time (s)')
    if poissonOutput:
        plot([0, endTime*1000],[0,0],':r',linewidth=1.0)    
        ylabel('Firing rate (Hz)')
    else:
        plot([0, endTime*1000],[coef*vt,coef*vt],':r',linewidth=1.5)    
        ylabel('Membrane potential (mV)')
        
    axis([minTime, maxTime, coef*(El-.1*(vt-El)), coef*(vt+.1*(vt-El))])
    leg = legend(loc='upper right')

#    title('B')
    idxPlot+=1
if monitorOutput:    
    # raster plots
    if useSubplot:
        idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
#        ax = fig.add_subplot(nRow,nCol,idxPlotTransp,xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)],yticks=outputSelection,yticklabels=[])
        ax = fig.add_subplot(nRow,nCol,idxPlotTransp)
        text(1.04, 0.5,'C', fontsize=20, horizontalalignment='center',verticalalignment='center',transform = ax.transAxes)
    else:
        fig = plt.figure()
#        ax = fig.add_subplot(1,1,1,title='Output spikes',xticks=[minTime,maxTime],xticklabels=[str(minTime/1000),str(maxTime/1000)])
        ax = fig.add_subplot(1,1,1,title='Output spikes')
        

    if len(outputSelection)<M: # color code for spikes
        for i in range(outputSpike.nspikes):
            if 1000*outputSpike.spikes[i][1]<minTime-10:
                continue
            if outputSpike.spikes[i][0] in outputSelection:
                plot([1000*outputSpike.spikes[i][1]],[outputSpike.spikes[i][0]],'.b',markersize=5) 
            if 1000*outputSpike.spikes[i][1]>maxTime+10:
                break
    else:
        raster_plot(outputSpike) 
        
    # just for easier visualization
    for i in arange(0,M,2*nG):
        rect = Rectangle( [minTime, i-.5], maxTime-minTime, nG, facecolor=[.8,.8,.8], edgecolor='none')
        ax.add_patch(rect)
        

   # pattern periods     
    if os.path.exists(os.path.join('..','data','patternPeriod.'+'%03d' % (randState)+'.mat')):
        patternPeriod=loadmat(os.path.join('..','data','patternPeriod.'+'%03d' % (randState)+'.mat'))
        # patternPeriod=(patternPeriod['patternPeriod']+timeOffset)
        patternPeriod=(patternPeriod['patternPeriod'])
        for i in range(size(patternPeriod,0)):
            if 1000*patternPeriod[i,1]<=minTime-10000:
                continue
            rect = Rectangle( [1000*patternPeriod[i,0], -1], 1000*(patternPeriod[i,1]-patternPeriod[i,0]), M+1, facecolor='grey', edgecolor='none')
            ax.add_patch(rect)
            if 1000*patternPeriod[i,0]>=maxTime+10000:
                break
     


    axis([minTime, maxTime, min(outputSelection)-5, max(outputSelection)+5])
#    axis([minTime, maxTime, 19.5, 21.5])
    xlabel('Time (s)')
#    ylabel('Neuron #')
#    ylabel('Increasing $w^{in}$')
    ylabel('Postsynaptic spikes')
    idxPlot+=1
    print str(outputSpike.nspikes) + ' post synaptic spikes'
    if outputSpike.nspikes>0:
        print 'Mean output firing rate = ' + '%.1f' % (outputSpike.nspikes/(endTime-outputSpike.spikes[0][1])/M)
        
#    print 'First output spikes: ' + str( outputSpike.spikes[0:min(5,outputSpike.nspikes)] )

    nu_avg = 20*Hz
    nu_out = - w_in * nu_avg / (w_out+(a_pre*tau_pre+a_post*tau_post)*nu_avg)
    print 'Theoretical mean output firing rate = ' + '%.1f' % mean(nu_out)
    
    print 'Output spikes done'

if monitorRate:    
    # firing rates
    if useSubplot:
        idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
        ax = fig.add_subplot(nRow,nCol,idxPlotTransp)
    else:
        fig = plt.figure()
        ax = fig.add_subplot(1,1,1)
    for i in outputSelection:
        f_pre = 10
        dwdt = f_pre*rate[i]._rate[0] * (a_pre*tau_pre/(1+tau_pre*(f_pre+rate[i]._rate[0]))+a_post[i]*tau_post/(1+tau_post*(f_pre+rate[i]._rate[0])) )
#        plot(rate[0].times/ms,rate[i].smooth_rate(6000*ms),label= str(i)+' (gmax=' + '%2.1e' % gmax[i] + ', r=' + '%.2f' % (a_post[i]/a_pre) + ', dW/dt='+'%2.1e' % dwdt)
        plot(rate[0].times/ms,rate[i]._rate,label= str(i)+' (gmax=' + '%2.1e' % gmax[i] + ', r=' + '%.2f' % (a_post[i]/a_pre) + ', dW/dt='+'%2.1e' % dwdt)
#    axis([minTime, 400*1000, 0, 90])
    xlabel('Time (in ms)')
    ylabel('Rates in Hz')
    leg = legend(loc='upper center')
    # matplotlib.text.Text instances
    for t in leg.get_texts():
        t.set_fontsize(8)    # the legend text fontsize

    # matplotlib.lines.Line2D instances
    for l in leg.get_lines():
        l.set_linewidth(1.5)  # the legend line width
    idxPlot+=1
    print 'Rates done'
if computeOutput:  
    # synaptic weights
    if useSubplot:
        idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
        ax = fig.add_subplot(nRow,nCol,idxPlotTransp,xticks=[0,1])
        text(1.04, 0.5,'D', fontsize=20, horizontalalignment='center',verticalalignment='center',transform = ax.transAxes)
    else:
        fig = plt.figure()
        ax = fig.add_subplot(1,1,1)
        
    # always show final synatpic weights
#    bar(histc(finalWeight,arange(0, 1.1, .1)))
    for i in outputSelection:
#        hist(finalWeight[:,i],bins=20,label= str(i)+' (gmax=' + '%2.1e' % gmax[i] + ', r=' + '%.2f' % (a_post[i]/a_pre) )
        hist(finalWeight[:,i],label= str(i)+' (gmax=' + '%2.1e' % gmax[i] + ', r=' + '%.2f' % (a_post[i]/a_pre) )
    axis([0, 1, 0, N])
#        bar(hist(finalWeight[:,i],bins=arange(0, 1.05, .05)),color=None,label= str(i)+' (gmax=' + '%2.1e' % gmax[i] + ', r=' + '%.1f' % (a_post[i]/a_pre) )
        
#    leg = legend()    
#    # matplotlib.text.Text instances
#    for t in leg.get_texts():
#        t.set_fontsize(8)    # the legend text fontsize
#    # matplotlib.lines.Line2D instances
#    for l in leg.get_lines():
#        l.set_linewidth(1.5)  # the legend line width
        
#    hist(x, bins=10, range=[0.,1.])
    xlabel('Normalized weight')
    ylabel('#')
    

    idxPlot+=1
    
    print "# of selected synapses=",str(sum(finalWeight>.5,0))    
    print "weight sum=",str(sum(finalWeight,0))    
    print 'Weight hist. done'
    
    if len(outputSelection)==1 and os.path.exists(os.path.join('..','data','realValuedPattern.'+'%03d' % (randState)+'.mat')):        
        if useSubplot:
            idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
            ax = fig.add_subplot(nRow,nCol,idxPlotTransp,xticks=[0,1])
            text(1.04, 0.5,'D', fontsize=20, horizontalalignment='center',verticalalignment='center',transform = ax.transAxes)
        else:
            fig = plt.figure()
            ax = fig.add_subplot(1,1,1)
        realValuedPattern=loadmat(os.path.join('..','data','realValuedPattern.'+'%03d' % (randState)+'.mat'),squeeze_me=True)
        realValuedPattern=realValuedPattern['realValuedPattern']
        plot(realValuedPattern,finalWeight[range(len(realValuedPattern)),outputSelection],'.')
        xlabel('Pattern value')
        ylabel('Normalized weight')
        axis([0, 1, -.1, 1.1])
        idxPlot+=1
    
#    if len(outputSelection)==1: # w=f(index) for academic patterns        
#        if useSubplot:
#            idxPlotTransp = mod(idxPlot-1,nRow)*nCol + 1 + (idxPlot-1)/nRow
#            ax = fig.add_subplot(nRow,nCol,idxPlotTransp,title='E',xticks=[0,1])
#        else:
#            fig = plt.figure()
#            ax = fig.add_subplot(1,1,1)
#        plot(range(N),finalWeight[range(size(realValuedPattern,1)),outputSelection],'.')
#        xlabel('Afferent #')
#        ylabel('Normalized weight')
#        axis([0, N-1, -.1, 1.1])
#        idxPlot+=1


#for i in range(M):
#for i in []:
for i in []:
    figure()
    hist(finalWeight[:,i],bins=20)
    title( str(i)+' (gmax=' + '%2.1e' % gmax[i] + ', r=' + '%.2f' % (a_post[i]/a_pre) )
    print 'Individual hist.' + str(i) +  ' done'
    
#show()

# for i in range(N-1):
#    print pot.mean[i]
#     print pot[i][len(pot[i])-1]
#     print pot[i][0]
#     print len(pot[i])
# print pot.times[len(pot.times)-1]

# write output in a file
# f = open("spikeList.txt", 'w')
# f.write("neuron spikeTime\n")
# for i in range(spike.nspikes-1):
#     f.write(str(spike.spikes[i][0])+" "+str(spike.spikes[i][1])+"\n")
# f.close()
