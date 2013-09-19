# save output for matlab
localStartTime = time()*second
print 'Dumping matlab data...'
outputdata={'weight':finalWeight,'firingTime':outputSpike.spikes}
savemat('../data/output.'+'%03d' % (randState)+'.mat',outputdata)
del outputdata
print 'Done in: '+ str(time()*second-localStartTime)                        
