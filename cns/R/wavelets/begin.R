require(wavethresh)


y <- c(1,1,7,9,2,8,8,6)

y = DJ.EX()$doppler


y= simchirp()$y
ywd = wd(y,filter.number=2, family='DaubExPhase', type='station')

#ywst = wst(y,filter.number=2, family='DaubExPhase')
#ymwd = mwd(y)

#plot(ywd, scaling="by.level")


#z = rnorm(256)
#zwp = wp(z, filter.number=2, family='DaubExPhase')
#plot(zwp)

#zwp2 = putpacket(zwp, level=6, index=3, packet = c(rep(0,10), 10, rep(0, 53) ))

#zwp2.nv = MaNoVe(zwp2)
chirpwdS <- wd(simchirp()$y, filter.number=4, family="DaubLeAsymm", type="station")
plot(chirpwdS)
#i=c(1:50)
#i=c(51:100)
#i=c(101:150)
#i=151:200
#i=201:250

#x = c(sapply(train_dataset[i], function(d) d$data))

fr_stat_plot = function(x) {
  par(mfrow=c(3,1))
  spectrum(x)
  plotl(x)
  plot(fft(x))
}

#x = x[1:2048]

#xwd = wd(x, filter.number=3, family='DaubExPhase', type='station')
#plot(xwd, scaling="by.level")
#plotl(accessD(xwd,level=1))

#library(wavelets)

#yy = modwt(simchirp()$y, filter="la8",10, boundary="reflection")
