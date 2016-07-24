# -*- coding: utf-8 -*-

import numpy as np
import librosa as lr
import logging
from os.path import expanduser as pp
from matplotlib import pyplot as plt
from os.path import join as pj
import os

def gauss(x, mean, sd):
    return np.exp( - (x - mean)*(x - mean)/sd)

ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
data_list_files = []
data_source = []
for f in sorted(os.listdir(ds_dir)):
    if f.endswith(".npy"):
        data_list_files.append(pj(ds_dir, f))
    if f.endswith(".wav"):
        data_source.append(pj(ds_dir, f))

source_id = 1


   
res_dir = pj(os.environ["HOME"], "Music", "ml", "impro_res")
if not os.path.exists(res_dir):
    os.makedirs(res_dir)

print "Opening ", data_list_files[source_id]
source = np.load(data_list_files[source_id]) 

n_fft = 2048 # default
n_mels = source.shape[0]
ms_frame = 10
sr = 22050
gauss_size = 1
gauss_sd = 1
hop = int(np.round(ms_frame * sr / 1000.0))

fftf = lr.fft_frequencies(sr=sr, n_fft=n_fft)
melf = lr.mel_frequencies(n_mels = n_mels)


dest = -60.0 * np.ones((len(fftf), source.shape[1]))

y, sr = lr.load(data_source[source_id])
stft = lr.stft(y, hop_length=hop)



#src_mask_real = np.mean(stft.real, 1)
#src_mask_imag = np.mean(stft.imag, 1)


# D = lr.logamplitude(np.abs(stft)**2, ref_power=np.max, top_db=60)

# src_bounds_real = np.min(D, 1), np.max(D, 1)
# src_bounds_imag = np.min(stft.imag, 1), np.max(stft.imag, 1)

stft = lr.stft(y, hop_length=hop)

s = stft.real
mel_basis = lr.filters.mel(sr, n_fft, n_mels)

for ri in xrange(s.shape[0]):
    s[ri, np.where(s[ri, :] < 0)] = 0.0
    s[ri, np.where(s[ri, :] < 0.5)] = 0.0
    s[ri, :] = np.log10(s[ri, :] + 1e-10)
    # denom = np.max(s[ri,:]) - np.min(s[ri,:])
    # if denom > 0:
    #     s[ri,:] = (s[ri,:] - np.min(s[ri,:]))/denom
    # else:
    #     s[ri,:] = 0

sm = np.dot(mel_basis, s)
s_rec = np.dot(mel_basis.T, sm)

for mi in xrange(n_mels):
    freq = melf[mi]
    df = np.abs(fftf - freq)
    fft_id = np.where(df == np.min(df))[0]
    if gauss_size == 1:
        data_real = src_bounds_real[0][fft_id] + source[mi,:] * (src_bounds_real[1][fft_id] - src_bounds_real[0][fft_id])
        # data_imag = src_bounds_imag[0][fft_id] + np.power(np.exp(source[mi,:]), 0.5) * (src_bounds_imag[1][fft_id] - src_bounds_imag[0][fft_id])
        # dest[fft_id, :] = 1j * data_imag
        dest[fft_id, :] =  data_real # np.power(np.exp(data_real), 0.5) 
    else:
        for filt_id in xrange(fft_id - gauss_size/2, fft_id + gauss_size/2):
            if filt_id >= fftf.shape[0]:
                continue
            data_real = src_bounds_real[0][filt_id] + source[mi, :] * gauss(filt_id, fft_id, sd=gauss_sd)* (src_bounds_real[1][filt_id] - src_bounds_real[0][filt_id])
            data_imag = src_bounds_imag[0][filt_id] + source[mi, :] * gauss(filt_id, fft_id, sd=gauss_sd)* (src_bounds_imag[1][filt_id] - src_bounds_imag[0][filt_id])
            # dest[filt_id, :] += 1j * data_imag
            dest[filt_id, :] += data_real 
            # dest[filt_id, :].real += src_bounds_real[0][filt_id] +  k * (src_bounds_real[1][filt_id] - src_bounds_real[0][filt_id])
            # dest[filt_id, :].imag += src_bounds_imag[0][filt_id] + k * (src_bounds_imag[1][filt_id] - src_bounds_imag[0][filt_id])
            # dest[filt_id, :] +=  np.complex(src_mask_real[filt_id] * k, src_mask_imag[filt_id] * k)


dest = np.power(10.0, (dest + 10*np.log10(np.max(np.abs(stft))))/10.0)
dest = dest ** 0.5

sign = np.ones(dest.shape)
sign[np.where(stft.real < 0)] = -1.0

dest *= sign

# dest = (dest.T * sign).T
# dest = np.power(10.0, dest)

# 10^x = S) 



plt.figure(1)
plt.subplot(2,1,1)
plt.imshow(stft.real[:,:5000])
plt.subplot(2,1,2)
plt.imshow(dest[:, :5000])
# plt.subplot(3,1,3)
# plt.imshow(dest.real[:, :5000] - stft.real[:, :5000])
plt.show()


y_r = lr.istft(dest, hop_length=hop)
lr.output.write_wav(pj(res_dir, "{}_result.wav".format(source_id)), y_r, sr)