import tensorflow as tf
import numpy as np
import os
from matplotlib import pyplot as plt
from os.path import join as pj
import sys
import cPickle as pkl
import gc
from model import gen_poisson, dispatch_array, recollect_array, dispatch_data, recollect_data

from mp_lib import SpikeRecords
from model import Corpus



ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
run_dir = pj(os.environ["HOME"], "Music", "ml", "impro_run")
if not os.path.exists(run_dir):
    os.makedirs(run_dir)



data_file_list = []

for f in sorted(os.listdir(ds_dir)):
    if f.endswith("spikes.pkl"):
        print "Considering {} as input".format(f)
        data_file_list.append(pj(ds_dir, f))

data_file_list = data_file_list[:3]

max_t, input_dims = 0, None

for inp_file in data_file_list:
    print "Reading {}".format(inp_file)
    spike_object = pkl.load(open(inp_file, "rb"))
    max_t = max(max_t, max(spike_object.spike_times))
    assert input_dims is None or input_dims == spike_object.neurons_num, "Got spikes with another neurons number"
    input_dims = spike_object.neurons_num

gc.collect()

batch_size = len(data_file_list)*10

bptt_steps = 50

corpus = Corpus(input_dims, len(data_file_list), batch_size, max_t, bptt_steps)

sfn=9
sf_sigma = 0.1
sf = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, sfn))/sf_sigma)

data_list = []

for inp_file in data_file_list:
    spike_object = pkl.load(open(inp_file, "rb"))
    tmax = max(spike_object.spike_times)
    neurons_num = spike_object.neurons_num
    
    data = np.zeros((neurons_num, tmax+1))
    for ni, fired_time in zip(spike_object.fired_neurons, spike_object.spike_times):
        data[ni, fired_time] = 1.0
            
    for ni in xrange(data.shape[0]):
        data[ni, :] = np.convolve(sf, data[ni, :], mode="same") 
        data[ni, :] = np.clip(data[ni, :], 0.0, 1.0)

    corpus.enrich_with_source(data)
    data_list.append(data)

d = recollect_data(corpus.data, corpus.seq_lengths, corpus.number_of_sources)