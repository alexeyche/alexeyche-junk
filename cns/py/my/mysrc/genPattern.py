#!/usr/bin/env python

import numpy

nAffer = 100

def spikeAvalanche(nAffer = 10, dt = 0.1, T = None):
    if T is None:
        T = nAffer*dt
    maxTime = nAffer*dt
    minTime = 0
    arr = None
    while maxTime <= T:
        if not arr == None:
            arr = numpy.concatenate( (arr, numpy.array([ numpy.arange(nAffer),numpy.linspace(minTime,maxTime-dt,nAffer)]).transpose() ))
        else:
            arr = numpy.array([ numpy.arange(nAffer),numpy.linspace(minTime,maxTime-dt,nAffer)]).transpose()
        minTime += nAffer*dt
        maxTime += nAffer*dt
    return arr

def spikeAvalancheBack(nAffer = 10, dt = 0.1, T = None):
    if T is None:
        T = nAffer*dt
    maxTime = nAffer*dt
    minTime = 0
    arr = None
    while maxTime <= T:
        if not arr == None:
            arr = numpy.concatenate( (arr, numpy.array([ numpy.arange(nAffer-1,-1,-1),numpy.linspace(minTime,maxTime-dt,nAffer)]).transpose() ))
        else:
            arr = numpy.array([ numpy.arange(nAffer-1,-1,-1),numpy.linspace(minTime,maxTime-dt,nAffer)]).transpose()
        minTime += nAffer*dt
        maxTime += nAffer*dt
    return arr
   
