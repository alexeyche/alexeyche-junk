import sys
import numpy as np


def restore(records, filters, time_from=0, given_s=None):
    tmax = max([ r[0] for r in records ]) - time_from    
    signal = np.zeros(tmax + filters.shape[1])
    for tup in records:
        t, filter_id = tup[0], tup[1]
        if given_s:
            s = given_s
        else:
            s = tup[2]
        t_id = t - time_from
        signal[t_id:(t_id+filters.shape[1])] += s * filters[filter_id, :]
    
    return signal/np.sqrt(np.sum(signal ** 2))

def gammatone(t, f, phase, ampl, n, b):
    return ampl * t ** (n-1) * np.exp(-2.0 * np.pi * b * t) * np.cos(2.0 * np.pi * f * t + phase)


def generate_dct_dictionary(size, l):
    p = np.asarray(xrange(l))
    filters = np.zeros((size, l))
    for fi in xrange(size):
        filters[fi, :] = np.cos((np.pi * (2 * fi + 1) * p)/(2*l))
        filters[fi, 0] *= 1.0/np.sqrt(2.0)
        # filters[fi, 1:] *= np.sqrt(2/l) 
    return filters * np.sqrt(2.0/l)

def run_on_batch(
    data_iter_start, 
    Rn, 
    true_signal, 
    filters, 
    match_iterations, 
    threshold, 
    learning,
    gpu_lambda,
    return_q
):
    print "Running on batch from t {}".format(data_iter_start)
    batch_records = []
    for _ in xrange(match_iterations):
        if gpu_lambda:
            prods = gpu_lambda(filters, Rn)
        else:
            prods = np.inner(filters, Rn)
        
        got_anybody = False
        filter_maxs = np.argmax(prods, 0)
        for bi, fi in enumerate(filter_maxs):
            if prods[fi, bi] >= threshold:
                Rn[bi, :] -= prods[fi, bi] * filters[fi,:]
                batch_records.append((data_iter_start + bi, fi, prods[fi, bi]))
                got_anybody = True
                
        if not got_anybody:
            break
    
    dfilters = np.zeros(filters.shape)

    if len(batch_records) > 0:
        if learning:
            restored_signal = restore(batch_records, filters, data_iter_start)
        
            signal_len = min(len(restored_signal), len(true_signal))
            restored_signal = restored_signal[:signal_len]
            true_signal = true_signal[:signal_len]

            for t, filter_id, s in batch_records:
                t_id = t - data_iter_start
                
                delta = true_signal[t_id:(t_id+filters.shape[1])] - restored_signal[t_id:(t_id+filters.shape[1])]
                dfilters[filter_id, :len(delta)] += s * delta
                    
            print "Done with batch from t {}, mean error {}".format(data_iter_start, np.mean((true_signal - restored_signal) ** 2))
        else:
            print "Done with batch from t {}, with {} spikes".format(data_iter_start, len(batch_records))
    
    else:
    	print "Done with batch from t {}, with no spikes".format(data_iter_start)

    return_q.put((batch_records, dfilters))
    sys.stdout.flush()



class SpikeRecords(object):
    def __init__(self, records, neurons_num, threshold, data_denominator, sr):
        self.spike_times, self.fired_neurons = [], []
        for r in records:
            self.spike_times.append(r[0])
            self.fired_neurons.append(r[1])
        self.threshold = threshold
        self.data_denominator = data_denominator
        self.sr = sr
        self.neurons_num = neurons_num

    def get_waveform(self, filters):
    	restored = restore(
            [ (spt, n) for spt, n in zip(self.spike_times, self.fired_neurons) ], 
            filters, 
            given_s=self.threshold
        )
        return restored * self.data_denominator, self.sr
        