
from scikits.statsmodels.tsa.arima_process import arma_generate_sample

from scipy import signal
import matplotlib.pyplot as plt

import pandas as pd

from util import *

def whiten(X,fudge=1E-18):

   # the matrix X should be observations-by-components

   # get the covariance matrix
   Xcov = np.dot(X.T,X)

   # eigenvalue decomposition of the covariance matrix
   d, V = np.linalg.eigh(Xcov)

   # a fudge factor can be used so that eigenvectors associated with
   # small eigenvalues do not get overamplified.
   D = np.diag(1. / np.sqrt(d+fudge))

   # whitening matrix
   W = np.dot(np.dot(V, D), V.T)

   # multiply by the whitening matrix
   X_white = np.dot(X, W)

   return X_white, W

def generate_ts(n, vol=0.3, lag=30):
    df = pd.DataFrame(np.random.randn(n) * np.sqrt(vol)).cumsum()
    df = df.rolling(window=lag, min_periods=1).mean()
    x = df.values[:,0]
    return (x - np.mean(x))/np.cov(x)

# def generate_ts(n):
# 	alphas = np.array([0.1, -0.1, 0.3, -0.1, 0.8])
# 	betas = np.array([0.5, -0.3, 0.1])

# 	ar = np.r_[1, -alphas]
# 	ma = np.r_[1, betas]

# 	return arma_generate_sample(ar=ar, ma=ma, nsample=n, burnin=1000)

def l2_norm(x, axis=0):
   return np.sqrt(np.maximum(np.sum(np.square(x), axis), 1e-10))


def filter_ts(x, W):
   filter_len = W.shape[0]
   Wn = l2_norm(W)

   x_w = np.zeros(x.shape)
   for ti in xrange(x.shape[0]):
      left_ti = max(0, ti - filter_len)
      xt_pad = np.concatenate([np.zeros((filter_len - (ti-left_ti))), x[left_ti:ti]])
      x_w[left_ti:ti] += np.dot(xt_pad, W)[:(ti-left_ti)]/Wn[:(ti-left_ti)]
   
   return x_w

def white_ts(x, filter_len, fudge=1e-12):
   C = np.zeros((filter_len, filter_len))
   for ti in xrange(x.shape[0]):
      left_ti = max(0, ti - filter_len)
      
      xt_pad = np.concatenate([
         np.zeros((filter_len - (ti-left_ti),)), 
         x[left_ti:ti]]
      )
      
      xt_norm = l2_norm(xt_pad)
      
      C += np.outer(xt_pad, xt_pad)/(xt_norm * xt_norm)

   C = C/x.shape[0]

   d, V = np.linalg.eigh(C)
   D = np.diag(1. / np.sqrt(d+fudge))
   W = np.dot(np.dot(V, D), V.T)
   
   return filter_ts(x, W), W


if __name__ == '__main__':
   np.random.seed(10)

   filter_len = 50
   fs = 10e2
   T = 1000
   fudge = 1e-18

   x = generate_ts(T)
   x_w, W = white_ts(x, filter_len)

   x_w2 = filter_ts(x_w, np.linalg.inv(W))

   shl(x, x_w2/500.0)
   shp(x_w)