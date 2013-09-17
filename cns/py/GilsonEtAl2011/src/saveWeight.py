# save weights
outputdata={'weight':finalWeight}
savemat('../data/weight.'+'%03d' % (randState)+'.mat',outputdata)
t=localtime()
savemat('../data/weight.'+'%03d' % (randState)+ '.%02d' % t[1] + '.%02d' % t[2] + '.%02d' % t[3] + '.%02d' % t[4] + '.%02d' % t[5] +'.mat',outputdata)
del outputdata
