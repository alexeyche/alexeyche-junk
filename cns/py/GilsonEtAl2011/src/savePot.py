# save pot
outputdata={'potential':pot.values, 'times':pot.times}
savemat('../data/potential.'+'%03d' % (randState)+'.mat',outputdata)

#t=localtime()
#savemat('../data/weight.'+'%03d' % (randState)+ '.%02d' % t[1] + '.%02d' % t[2] + '.%02d' % t[3] + '.%02d' % t[4] +'.mat',outputdata)

# save output spikes
outputdata={'outputSpike':outputSpike.spikes}
savemat('../data/outputSpike.'+'%03d' % (randState)+'.mat',outputdata)

del outputdata
