#!/usr/bin/RScript

source("serv.R")

system("../build/bin/srm_test -t epsp")

load()
plot(epsp[,1], epsp[,2], type="l")

system("../build/bin/srm_test -t nu")

load()
plot(nupot[,1], nupot[,2], type="l")

system("../build/bin/srm_test -t srm")
load()
plot(pot[,1], pot[,2], type="l")
