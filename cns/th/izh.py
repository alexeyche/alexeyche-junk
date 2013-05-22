#!/usr/bin/PScript

import theano
import theano.tensor as T
import numpy as np



a=0.02
b=0.2
c=-65
d=6

V0 = -70
u0=b*V0
tau=0.25

sim_time=100 # msec
I = 14

a = T.cast(a, dtype=theano.config.floatX)
b = T.cast(b, dtype=theano.config.floatX)
c = T.cast(c, dtype=theano.config.floatX)
d = T.cast(d, dtype=theano.config.floatX)
V0 = T.cast(V0, dtype=theano.config.floatX)
u0 = T.cast(u0, dtype=theano.config.floatX)
tau = T.cast(tau, dtype=theano.config.floatX)
I = T.cast(I, dtype=theano.config.floatX)



class Neuron():
    def __init__(self):
        self.V = V0
        self.u = u0
        self.Isyn = I

    def calc(self):
        V = self.V + tau*(0.04*T.sqr(self.V) + 5.*self.V + 140. - self.u + self.Isyn)
        u = self.u + tau*a*(b*self.V-self.u)
        self.V = T.switch(T.gt(V, 30), c, V)
        self.u = T.switch(T.gt(V, 30), u+d, u)
        return V, u


n = Neuron()

(VV, uu), updates = theano.scan(n.calc, n_steps=1200)

f = theano.function([], [VV, uu], updates=updates)


