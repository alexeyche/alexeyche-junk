#!/usr/bin/env sage

from sage.all_cmdline import *

load_attach_path("/home/alexeyche/prog/ns/openket")
load('openket.sage')


print qmatrix(Z())

def My(A = 'default'):
    return Ket(1,A) Bra(1,A)

print qmatrix(My())
