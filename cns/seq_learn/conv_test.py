

import os
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj


def conv_forward_naive(x, w, b, conv_param):
  """
  A naive implementation of the forward pass for a convolutional layer.

  The input consists of N data points, each with C channels, height H and width
  W. We convolve each input with F different filters, where each filter spans
  all C channels and has height HH and width WW.

  Input:
  - x: Input data of shape (N, C, H, W)
  - w: Filter weights of shape (F, C, HH, WW)
  - b: Biases, of shape (F,)
  - conv_param: A dictionary with the following keys:
    - 'stride': The number of pixels between adjacent receptive fields in the
      horizontal and vertical directions.
    - 'pad': The number of pixels that will be used to zero-pad the input.

  Returns a tuple of:
  - out: Output data, of shape (N, F, H', W') where H' and W' are given by
    H' = 1 + (H + 2 * pad - HH) / stride
    W' = 1 + (W + 2 * pad - WW) / stride
  - cache: (x, w, b, conv_param)

  """

  N,C,H,W = x.shape 
  F,C,HH,WW = w.shape
  stride, pad = conv_param['stride'], conv_param['pad']
  H_prime = 1. + float(H + 2 * pad - HH) / float(stride)
  W_prime = 1. + float(W + 2 * pad - WW) / float(stride)
  assert H_prime % 1 == 0
  assert W_prime % 1 == 0
  H_prime,W_prime = int(H_prime), int(W_prime)

  # pad input array
  x_padded = np.pad(x, ((0,0), (0,0), (pad, pad), (pad, pad)), 'constant')
  H_padded, W_padded = x_padded.shape[2], x_padded.shape[3]
  # naive implementation of im2col
  x_cols = None
  for i in xrange(HH, H_padded+1, stride):
    for j in xrange(WW, W_padded+1, stride):
      for n in xrange(N):
        field = x_padded[n,:,i-HH:i, j-WW:j].reshape((1,C*HH*WW))    
        if x_cols is None:
            x_cols = field
        else:
            x_cols = np.vstack((x_cols, field))
  
  # x_cols shape: (HH * WW * C) x (H_prime * W_prime * N)
  x_cols = x_cols.T
  print x_cols.shape
  #w2col, get w into shape of (F) x (HH * WW * C) 
  w_cols = w.reshape((F, C*HH *WW))
  
  
  # out_cols shape = (F) x (H_prime * W_prime * N)
  out_cols = np.dot(w_cols, x_cols) + b.reshape((b.shape[0],1))

  # out shape = N x F x H' x W'
  out = out_cols.reshape(F, H_prime, W_prime, N)
  out = out.transpose(3, 0, 1, 2) # (N, F, H', W')

  cache = (x, w, b, conv_param)
  return out, cache


def conv_backward_naive(dout, cache):
  """
  A naive implementation of the backward pass for a convolutional layer.

  Inputs:
  - dout: Upstream derivatives. (N, F, H', W')
  - cache: A tuple of (x, w, b, conv_param) as in conv_forward_naive

  Returns a tuple of:
  - dx: Gradient with respect to x
  - dw: Gradient with respect to w
  - db: Gradient with respect to b
  
  - x: Input data of shape (N, C, H, W)
  - w: Filter weights of shape (F, C, HH, WW)
  - b: Biases, of shape (F,)
  - out: Output data, of shape (N, F, H', W') where H' and W' are given by
    H' = 1 + (H + 2 * pad - HH) / stride
    W' = 1 + (W + 2 * pad - WW) / stride
  
  """
  dx, dw, db = None, None, None
  x, w, b, conv_param = cache
  stride, pad = conv_param['stride'], conv_param['pad']
  N,C,H,W = x.shape 
  F,C,HH,WW = w.shape
 
  H_prime = 1. + float(H + 2 * pad - HH) / float(stride)
  W_prime = 1. + float(W + 2 * pad - WW) / float(stride)
  assert H_prime % 1 == 0
  assert W_prime % 1 == 0
  H_prime,W_prime = int(H_prime), int(W_prime)

  db = np.sum(dout, (0, 2, 3)) # sum along axis N, H', and W'
  
  # pad input array
  x_padded = np.pad(x, ((0,0), (0,0), (pad, pad), (pad, pad)), 'constant')
  H_padded, W_padded = x_padded.shape[2], x_padded.shape[3]
  # naive implementation of im2col
  x_cols = None
  for i in xrange(HH, H_padded+1, stride):
    for j in xrange(WW, W_padded+1, stride):
      for n in xrange(N):
        field = x_padded[n,:,i-HH:i, j-WW:j].reshape((1,C*HH*WW))    
        if x_cols is None:
            x_cols = field
        else:
            x_cols = np.vstack((x_cols, field))
  # x_cols shape: (HH * WW * C) x (H' * W' * N)
  x_cols = x_cols.T
  
  dout_ = dout.transpose(1, 2, 3, 0) # (F, H', W', N)
  dout_cols = dout_.reshape(F, H_prime * W_prime * N) # (F) x (H' * W' * N)

  dw_cols = np.dot(dout_cols, x_cols.T) # (F) x (HH * WW * C) 
  dw = dw_cols.reshape(F, C, HH, WW) # (F, C, HH, WW)

  w_cols = w.reshape(F, C*HH*WW) # (F) x (HH * WW * C)
  print w_cols.T.shape, dout_cols.shape
  dx_cols = np.dot(w_cols.T, dout_cols) # (HH * WW * C) x (H' * W' * N)
  
  # col2im: convert back from (d)x_cols to (d)x
  #dx = col2im_indices(dx_cols, (N, C, H, W), HH, WW, pad, stride)
  #dx_cols = dx_cols.T # (H' * W' * N) x (HH * WW * C)
  dx_padded = np.zeros((N, C, H_padded, W_padded))
  idx = 0
  for i in xrange(HH, H_padded+1, stride):
    for j in xrange(WW, W_padded+1, stride):
      for n in xrange(N):
        dx_padded[n:n+1,:,i-HH:i,j-WW:j] += dx_cols[:,idx].reshape(1,C,HH,WW)
        idx += 1
  dx = dx_padded[:,:,pad:-pad,pad:-pad]
  return dx, dw, db


seq_size = 1000
L = 100
N = 1
C = 1
H = 1
W = seq_size
F = 10

input_v = np.asarray([0.0] * seq_size)
input_v[50] = 1.0
input_v[500] = 1.0
input_v[750] = 1.0

x = input_v.reshape(N, C, H, W)



filter = np.asarray([np.exp(-np.linspace(0.0, 1.0, L)/0.1)]*F)
w = filter.reshape(F, C, H, L)
b = np.zeros(F)

conv_param = {}
conv_param["stride"] = 1
conv_param["pad"] = 1


out, cache = conv_forward_naive(x, w, b, conv_param)
# dx, dw, db = conv_backward_naive(out, (x, w, b, conv_param))

def im2col(x, num, stride=1):
    x_size = x.shape[0]
    x_cols = None
    for i in xrange(num, x_size+1, stride):
        field = x[i-num:i]
        if x_cols is None:
            x_cols = field
        else:
            x_cols = np.vstack((x_cols, field))
    return x_cols


r = im2col(input_v, L)

c = np.dot(r, filter.T)