#!/usr/bin/env python

import numpy as np
import glob
import os
from util import *


# Setup:
# 0. Feedforward and recurrent random connections with real number weights
# 1. Simple 1-threshold activation function
# 2. Simple Hebb's learning rule to correlatie co-occurences:
#      if Apre == 1 and Apost == 1:
#          dW += constant
#      if Apre == 1 and Apost == 0:
#          dW += 0.1 * constant   # ???
#      if Apre == 0 and Apost == 1:
#          dW -= constant
# 3. l2 weights normalization
# 4. Multiple layers   # ???


# Expectations:
# 0. Learned pattern will have distinctive and excitatory properties
# 1. Non seen pattern will be noisy and not distinctive
# 2. After some time (to figure out) Non seen pattern started to be distinctive
# 3. Correlate distance measure on input sequences and patterns?

def sparse_w(shape, p):
    W = np.random.random(shape)
    W[np.random.random(shape) > p] = 0.0
    return W

def norm(W, p=1):
    Wn = np.linalg.norm(W, p, axis=0)
    Wpos = np.where(Wn > 0)[0]
    W[:, Wpos] /= Wn[Wpos]
    return W


def cooc(Apre, Apost, W, dW):
    for pre in range(W.shape[0]):
        for post in range(W.shape[1]):
            if Apre[pre] == 1.0 and Apost[post] == 1.0:
                dW[pre, post] += 0.1
            elif Apre[pre] == 1.0 and Apost[post] == 0.0:
                pass
            elif Apre[pre] == 0.0 and Apost[post] == 1.0:
                dW[pre, post] -= 0.01
    return dW


np.random.seed(11)

num_inputs = 25
num_neurons = 100
num_iters = 25
threshold = 0.01
dt = 0.1

def act(U):
    A = np.zeros(U.shape)
    A[np.where(U > threshold)] = 1.0
    return A


X = np.zeros((num_iters, num_inputs))

for i in range(num_iters):
    # input_id = int(num_inputs * i / num_iters)
    input_id = i - num_inputs * (i // num_inputs)
    X[i, input_id] = 1.0


Wff = sparse_w((num_inputs, num_neurons), 0.05)
Wrec = sparse_w((num_neurons, num_neurons), 0.05)

Wff = norm(Wff)
Wrec = norm(Wrec)

Wff0 = Wff.copy()
Wrec0 = Wrec.copy()

epochs = 100
Ag = np.zeros((epochs, num_iters, num_neurons,))
for e in range(epochs):
    Ah = np.zeros((num_iters, num_neurons,))
    Uh = np.zeros((num_iters, num_neurons,))

    A = np.zeros((num_neurons,))
    U = np.zeros((num_neurons,))

    dWff = np.zeros(Wff.shape)
    dWrec = np.zeros(Wrec.shape)

    for i in range(num_iters):
        U += dt * (np.dot(X[i], Wff) - np.dot(A, Wrec) - U)
        A = act(U)

        Ah[i] = A.copy()
        Uh[i] = U.copy()

        cooc(X[i], A, Wff, dWff)
        cooc(A, A, Wrec, dWrec)

    # dWff = np.dot(X.T, Ah) / num_iters
    # dWrec = np.dot(Ah.T, Ah) / num_iters

    Wff += 0.1 * dWff
    Wff = norm(Wff)
    Wrec += 0.1 * dWrec
    Wrec = norm(Wrec)

    Ag[e] = Ah.copy()

    print("Epoch {}, |dA| + {:.4f}, |dWff| = {:.4f}, |dWrec| = {:.4f}".format(
        e,
        (np.linalg.norm(Ag[e - 1] - Ag[e]) if e > 0 else 0.0),
        np.linalg.norm(dWff),
        np.linalg.norm(dWrec),
    ))

# [os.remove(f) for f in glob.glob("/Users/aleksei/tmp/pics/*.png")]
# for e in range(epochs):
#     shm(Ag[e], file="/Users/aleksei/tmp/pics/{}.png".format(e))

# shl(Wff[:,5], Ah[:,5], dWff[:,5])

