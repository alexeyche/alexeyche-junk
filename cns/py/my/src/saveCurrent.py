# save pot
outputdata={'current':current.values, 'times':current.times}
savemat('../data/current.'+'%03d' % (randState)+'.mat',outputdata)

del outputdata
