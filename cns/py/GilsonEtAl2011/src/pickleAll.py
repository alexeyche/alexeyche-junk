# Dump all variables
# Under development...

printtime('************')
printtime('* Pickling *')
printtime('************')

# open pickle file for writing
f = open(os.path.join('..','data','output.'+'%03d' % (randState)+'.pkl'), 'w')

pickle.dump(endTime,f)
if monitorInput:
    localStartTime = time()*second
    print 'Pickling input spikes...'
    pickle.dump(inputSpike,f)
    print 'Done in: '+ str(time()*second-localStartTime)                        
if computeOutput:  
    localStartTime = time()*second
    print 'Pickling final weights...'
    pickle.dump(finalWeight,f)
    print 'Done in: '+ str(time()*second-localStartTime)                        
if monitorOutput:    
    localStartTime = time()*second
    print 'Pickling output spikes...'
    pickle.dump(outputSpike,f)    
    print 'Done in: '+ str(time()*second-localStartTime)                        
if monitorPot:    
    localStartTime = time()*second
    print 'Pickling membrane potential...'    
    pickle.dump(pot,f)
    print 'Done in: '+ str(time()*second-localStartTime)                            
if monitorCurrent:
    localStartTime = time()*second
    print 'Pickling current...'    
    pickle.dump(current,f)    
    print 'Done in: '+ str(time()*second-localStartTime)                            
if monitorRate:
    localStartTime = time()*second
    print 'Pickling rates...'    
    pickle.dump(rate,f)    
    print 'Done in: '+ str(time()*second-localStartTime)                            
# close pickle file
f.close()
