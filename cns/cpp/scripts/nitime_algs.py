#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt

import nitime.utils as utils
import nitime.timeseries as ts
import nitime.viz as viz
from nitime import algorithms as tsa
import nitime.utils as tsu

signal = np.loadtxt(open("../build/signal.csv","rb"),delimiter=",",skiprows=1)

restored = np.loadtxt(open("../build/restored.csv","rb"),delimiter=",",skiprows=1)
inp_sampling_rate = 1000.
lb = 0 # Hz
ub = 1000 # Hz

_, signal_spectra, _ = tsa.multi_taper_psd(signal,
                            Fs=inp_sampling_rate,
                            BW=None,
                            adaptive=True,
                            low_bias=True)
_, noise_spectra, _ = tsa.multi_taper_psd(restored-signal,
                            Fs=inp_sampling_rate,
                            BW=None,
                            adaptive=True,
                            low_bias=True)


freqs = np.linspace(0, inp_sampling_rate / 2, signal.shape[-1] / 2 + 1)

f = plt.figure()
ax = f.add_subplot(1, 2, 1)
ax_snr_info = f.add_subplot(1, 2, 2)
lb_idx, ub_idx = tsu.get_bounds(freqs, lb, ub)
freqs = freqs[lb_idx:ub_idx]
snr = signal_spectra/noise_spectra

ax.plot(freqs, np.log(signal_spectra[lb_idx:ub_idx]), label='Signal')
ax.plot(freqs, np.log(noise_spectra[lb_idx:ub_idx]), label='Noise')

ax_snr_info.plot(freqs, snr[lb_idx:ub_idx], label='SNR')
ax_snr_info.plot(np.nan, np.nan, 'r', label='Info')
ax_snr_info.set_ylabel('SNR')
ax_snr_info.set_xlabel('Frequency (Hz)')

ax.set_xlabel('Frequency (Hz)')
ax.set_ylabel('Spectral power (dB)')

plt.show()

