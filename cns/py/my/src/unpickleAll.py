# Load all previously dumped variables
# Under development...

printtime('**************')
printtime('* Unpickling *')
printtime('**************')

# open pickle file
f = open(os.path.join('..','data','output.'+'%03d' % (randState)+'.pkl'), 'r')
endTime = pickle.load(f)
if monitorInput:
    print 'Unpickling inputSpike'
    inputSpike=pickle.load(f)
if computeOutput:  
    print 'Unpickling finalWeight'
    finalWeight=pickle.load(f)
if monitorOutput:    
    print 'Unpickling outputSpike'
    outputSpike=pickle.load(f)
if monitorPot:    
    print 'Unpickling potential'
    pot=pickle.load(f)
if monitorCurrent:    
    print 'Unpickling current'
    current=pickle.load(f)
if monitorRate:    
    print 'Unpickling rate'
    rate=pickle.load(f)
f.close()

