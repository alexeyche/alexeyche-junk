#!/usr/bin/env python

import os
import tensorflow as tf
import numpy as np
from tensorflow.python.ops.rnn_cell import RNNCell
from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import init_ops
from tensorflow.python.ops.math_ops import tanh
from tensorflow.python.ops.math_ops import sigmoid

import math
import scipy
import scipy.sparse

def gauss_act(x, sigma):
    return tf.exp( - tf.square(-1.0 - tf.cos(x) )/( 2.0 * sigma ** 2))

def laplace_act(x, sigma):
    return tf.exp( - tf.abs(tf.cos(x) + 1.0)/( 2.0 * sigma ** 2))

def epsp_act(x, sigma):
    return tf.exp( - (tf.cos(x) + 1.0)/( 2.0 * sigma ** 2))

def simple_act(x, sigma):
    return -tf.cos(x)/2.0 + 0.5


class ThetaRNNCell(RNNCell):
    """Theta neuron RNN cell."""

    def __init__(
        self,
        num_units,
        dt,
        activation = simple_act,
        input_weights_init = tf.uniform_unit_scaling_initializer(factor=1.0),
        recc_weights_init = tf.uniform_unit_scaling_initializer(factor=1.0),
        sigma = None,
        update_gate = False
    ):
        self._num_units = num_units
        self._activation = activation
        self._dt = dt
        self._sigma = sigma if sigma else 1.0
        self._update_gate = update_gate

        self.W = None
        self.U = None
        self.bias = None
        self.W_u = None
        self.U_u = None
        self.bias_u = None
        
        self.input_weights_init = input_weights_init
        self.recc_weights_init = recc_weights_init

        self.states_info = []

    @property
    def state_size(self):
        return self._num_units

    @property
    def output_size(self):
        return self._num_units

    def __call__(self, inputs, state, scope=None):
        with vs.variable_scope(scope or type(self).__name__):
            batch_size = inputs.get_shape().with_rank(2)[0]
            input_size = inputs.get_shape().with_rank(2)[1]

            self.W = vs.get_variable("W", [input_size, self._num_units], initializer=self.input_weights_init)
            self.U = vs.get_variable("U", [self._num_units, self._num_units], initializer=self.recc_weights_init)
            self.bias = vs.get_variable("Bias", [self._num_units], initializer=init_ops.constant_initializer(0.0))

            state_cos = tf.cos(state)
            weighted_input =  math_ops.matmul(inputs, self.W) + math_ops.matmul(state, self.U) + self.bias

            new_state = 1.0 - state_cos + (1.0 + state_cos) * weighted_input
            if not self._update_gate:
                state = state + self._dt * new_state
            else:
                self.W_u = vs.get_variable("W_u", [input_size, self._num_units], initializer=self.input_weights_init)
                self.U_u = vs.get_variable("U_u", [self._num_units, self._num_units], initializer=self.recc_weights_init)
                self.bias_u = vs.get_variable("Bias_u", [self._num_units], initializer=init_ops.constant_initializer(0.0))
                u = sigmoid(math_ops.matmul(inputs, self.W_u) + math_ops.matmul(state, self.U_u) + self.bias_u)
                state = u * state + (1.0-u) * self._dt * new_state

            output = self._activation(new_state, self._sigma)

            self.states_info.append(new_state)
        return output, state


def gen_poisson(rates, T, dt, signal_form=None, seed=None):
    if seed:
        np.random.seed(seed)
    res = dt * rates >  np.random.random((T, rates.shape[0]))
    res = res.astype(np.float32)
    if signal_form is not None:
        for ni in xrange(len(rates)):
            res[:, ni] = np.convolve(signal_form, res[:, ni], mode="same")
            res[:, ni] = np.clip(res[:, ni], 0.0, 1.0)
    return res


def dispatch_array(data, seq_size, batch_size):
    corpus = []
    batch_len = data.shape[1]/batch_size
    batch_ids = np.asarray(range(0, data.shape[1], batch_len))
    batch_ends_ids = np.clip(batch_ids + batch_len, 0, data.shape[1])
    # print batch_len, batch_ends_ids

    we_are_done = False
    while not we_are_done:
        inputs_v = []
        # print len(corpus), batch_ids
        for si in xrange(seq_size):
            ds = np.zeros((len(batch_ids), data.shape[0]))
            data_presence = np.where(batch_ids < batch_ends_ids)[0]
            if len(data_presence) == 0:
                we_are_done = True
                if si > 0:
                    inputs_v.append(ds)
                    continue
                else:
                    break
            ds[data_presence, :] = data[:, batch_ids[data_presence]].T
            inputs_v.append(ds)
            batch_ids += 1
        if len(inputs_v)>0:
            corpus.append(inputs_v)
    return corpus

def recollect_array(corpus):
    assert (len(corpus) > 0) and (len(corpus[0]) > 0), "Got empty data"

    data_len = sum([ subc.shape[0] for c in corpus for subc in c ])
    dim_size = corpus[0][0].shape[1]
    batch_size = corpus[0][0].shape[0]
    data = np.zeros((dim_size, data_len))

    batch_len = data_len/batch_size
    batch_ids = np.asarray(range(0, data_len, batch_len))
    for c in corpus:
        for seq in c:
            data[:, batch_ids] = seq.T
            batch_ids += 1
    return data


class Corpus(object):
    def __init__(self, input_dimension, number_of_sources, batch_size, max_t, seq_size, data_end=None):
        self.batch_size = batch_size
        self.max_t = max_t
        self.seq_size = seq_size
        self._source_id = 0
        self.number_of_sources = number_of_sources
        self.sub_batch_size = self.batch_size/self.number_of_sources
        self.input_dimension = input_dimension
        
        assert self.batch_size % self.number_of_sources == 0, "Need batch size be denominator of number of sources"
        self.sub_batch_num = self.batch_size/self.sub_batch_size
        
        per_seq = self.max_t / self.seq_size + self.max_t % self.seq_size
        per_batch = per_seq / self.sub_batch_size + per_seq % self.sub_batch_size
        self.data = list( 
            list(
                scipy.sparse.csr_matrix((self.batch_size, self.input_dimension), dtype=np.float32) 
                for si in xrange(self.seq_size)
            ) 
            for ci in xrange(per_batch) 
        )
        self.data_end = data_end if not data_end is None else np.zeros(self.batch_size, dtype=np.int32)
    
    @staticmethod
    def construct_from(other_corpus, dim=None):
        return Corpus(
            other_corpus.input_dimension if dim is None else dim, 
            other_corpus.number_of_sources, 
            other_corpus.batch_size, 
            other_corpus.max_t, 
            other_corpus.seq_size,
            other_corpus.data_end
        )

    def feed_corpus(self, corpus_id, corpus):
        assert corpus_id < len(self.data), "Out of corpus index"
        for seq_id, seq_data in enumerate(corpus):
            self.data[corpus_id][seq_id] = scipy.sparse.csc_matrix(seq_data)


    def feed_batch(self, corpus_id, seq_id, batch):
        assert corpus_id < len(self.data), "Out of corpus index"
        assert seq_id < self.seq_size, "Out of seq size"
        self.data[corpus_id][seq_id] = scipy.sparse.csc_matrix(batch)
        
    def enrich_with_source(self, data):
        assert self._source_id < self.number_of_sources, "Got too many data sources"
        
        batch_ids = np.zeros(self.sub_batch_size, dtype=np.int32)
        batch_ends_ids = np.zeros(self.sub_batch_size, dtype=np.int32)
        data_len = data.shape[0]

        batch_ends_ids[0] = data_len % self.sub_batch_size
        batch_ends_ids[0] += data_len / self.sub_batch_size
        self.data_end[self._source_id * self.sub_batch_size] = batch_ends_ids[0]
        for b_idx in xrange(1, self.sub_batch_size):
            batch_ids[b_idx] = batch_ends_ids[b_idx-1]
            sl = data_len / self.sub_batch_size
            batch_ends_ids[b_idx] = batch_ids[b_idx] + sl 
            self.data_end[self._source_id * self.sub_batch_size + b_idx] = sl

        full_set = frozenset(range(0, self.sub_batch_size))
        dst_batch = np.zeros(self.batch_size, dtype=np.bool)
        for bi in xrange(self.batch_size):
            if bi >= self._source_id * self.sub_batch_size and bi < (self._source_id+1) * self.sub_batch_size:
                dst_batch[bi] = True

        for ci in xrange(len(self.data)):
            for si in xrange(self.seq_size):
                data_presence = np.where(batch_ids < batch_ends_ids)[0]
                if len(data_presence) == 0:
                    self._source_id += 1
                    return
                for dp_i in full_set - set(data_presence):
                    dst_batch[self._source_id * self.sub_batch_size + dp_i] = False
                
                self.data[ci][si][dst_batch, :] = data[batch_ids[data_presence], :]
                batch_ids += 1
        self._source_id += 1
    
    def _get_source_batch_ids(self, source_id, dst_batch_ids):
        assert source_id < self.sub_batch_num, "Trying to recollect something that not present in corpus"
        
        source_b_ids = np.zeros(self.sub_batch_size, dtype=np.int32)
        source_b_ids[0] = dst_batch_ids[0]
        cumul_end = self.data_end[source_id * self.sub_batch_size]
        for di in xrange(1, self.sub_batch_size): 
            source_b_ids[di] = dst_batch_ids[di] + cumul_end
            cumul_end += self.data_end[source_id * self.sub_batch_size + di]
        return source_b_ids

    def recollect(self, id, head=None):
        assert id < self.sub_batch_num, "Trying to recollect something that not present in corpus"
        
        batch_masks =[]
        for source_id in xrange(self.sub_batch_num):
            dst_batch = np.zeros(self.batch_size, dtype=np.bool)
            for bi in xrange(self.batch_size):
                if bi >= source_id * self.sub_batch_size and bi < (source_id+1) * self.sub_batch_size:
                    dst_batch[bi] = True
            batch_masks.append(dst_batch)

        full_set = frozenset(range(0, self.sub_batch_size))
        batch_ids = np.zeros(self.sub_batch_size, dtype=np.int32)
        batch_ends_ids = self._get_source_batch_ids(
            id, 
            self.data_end[(id * self.sub_batch_size):(id * self.sub_batch_size + self.sub_batch_size)]
        )

        dst = np.zeros((head if head else sum(self.data_end[batch_masks[id]]), self.input_dimension))
        for ci in xrange(len(self.data)):
            for si in xrange(self.seq_size):
                source_ids = self._get_source_batch_ids(id, batch_ids)
                
                for b_id, (current_id, end_id) in enumerate(zip(source_ids.copy(), batch_ends_ids)):
                    if current_id >= end_id or (head and current_id >= head):
                        batch_masks[id][id * self.sub_batch_size + b_id] = False
                 
                bm = batch_masks[id][(id*self.sub_batch_size):(id+1)*self.sub_batch_size]
                if not np.any(bm):
                    return dst

                source_ids_left = source_ids[bm]
                
                dst[source_ids_left, :] = self.data[ci][si][batch_masks[id], :].todense()
                batch_ids += 1
        return dst 
                
    def prepare_sequence(self, corpus_id, shift=0):
        assert corpus_id < len(self.data), "Corpus id out of length"
        res = []
        for seq_id in xrange(shift, self.seq_size+shift):
            shifted_seq_id = seq_id % self.seq_size
            if seq_id == self.seq_size:
                corpus_id += 1
                if corpus_id >= len(self.data):
                    corpus_id = 0
            res.append(self.data[corpus_id][shifted_seq_id].todense())
        return res            
    

    @property
    def shape(self):
        return (len(self.data), self.seq_size, self.batch_size, self.input_dimension)
    

def dispatch_data(data_list, seq_size, batch_size=None):
    if batch_size is None:
        batch_size = len(data_list)
    assert len(data_list)>0, "Got empty data list"
    assert batch_size % len(data_list) == 0, "Batch size is not denominator of data list length"
    
    dim = data_list[0].shape[0]
    sub_batch_size = batch_size/len(data_list)
    batch_ids = np.zeros(batch_size, dtype=np.int32)
    batch_ends_ids = np.zeros(batch_size, dtype=np.int32)
    
    b_idx = 0
    for d in data_list:
        batch_ends_ids[b_idx] = d.shape[1] % sub_batch_size 
        for _ in xrange(0, sub_batch_size):
            batch_ends_ids[b_idx] += d.shape[1]/sub_batch_size
            b_idx += 1
    
    corpus = []

    we_are_done = False
    corp_idx = 0
    while not we_are_done:
        inputs_v = []
        print len(corpus), batch_ids
        for si in xrange(seq_size):
            ds = np.zeros((len(batch_ids), dim))
            data_presence = np.where(batch_ids < batch_ends_ids)[0]
            if len(data_presence) == 0:
                we_are_done = True
                if si > 0:
                    inputs_v.append(ds)
                    continue
                else:
                    break
    
            data_presence_set = set(data_presence)
            for di, d in enumerate(data_list):
                subb_cum_len = 0
                for sub_batch_id in xrange(0, sub_batch_size):
                    batch_id = di * sub_batch_size + sub_batch_id
                    if batch_id in data_presence_set:
                        ds[batch_id, :] = d[:, subb_cum_len + batch_ids[di]]
                    subb_cum_len += batch_ends_ids[batch_id]
                    
            inputs_v.append(ds)
            batch_ids += 1
        if len(inputs_v)>0:
            corpus.append(inputs_v)
    return corpus, batch_ends_ids

# def dispatch_spikes(spikes_list, seq_size, batch_size=None):
#     if batch_size is None:
#         batch_size = len(spikes_list)
#     assert len(spikes_list)>0, "Got empty spikes list"
#     assert batch_size % len(spikes_list) == 0, "Batch size is not denominator of data list length"
    
#     dim = spikes_list[0].neurons_num
#     sub_batch_size = batch_size/len(spikes_list)
#     batch_ids = np.zeros(batch_size, dtype=np.int32)
#     batch_ends_ids = np.zeros(batch_size, dtype=np.int32)
    
#     b_idx = 0
#     for d in spikes_list:
#         tmax = len(d.spike_times)
#         batch_ends_ids[b_idx] = tmax % sub_batch_size 
#         for _ in xrange(0, sub_batch_size):
#             batch_ends_ids[b_idx] += tmax/sub_batch_size
#             b_idx += 1
    
#     corpus = []

#     we_are_done = False
#     while not we_are_done:
#         inputs_v = []
#         print len(corpus), batch_ids
#         for si in xrange(seq_size):
#             ds = np.zeros((len(batch_ids), dim))
#             data_presence = np.where(batch_ids < batch_ends_ids)[0]
#             if len(data_presence) == 0:
#                 we_are_done = True
#                 if si > 0:
#                     inputs_v.append(ds)
#                     continue
#                 else:
#                     break
    
#             data_presence_set = set(data_presence)
#             for di, d in enumerate(spikes_list):
#                 subb_cum_len = 0
#                 for sub_batch_id in xrange(0, sub_batch_size):
#                     batch_id = di * sub_batch_size + sub_batch_id
#                     if batch_id in data_presence_set:
                        
#                         ds[batch_id, :] = d[:, subb_cum_len + batch_ids[di]]
#                     subb_cum_len += batch_ends_ids[batch_id]
                    
#             inputs_v.append(ds)
#             batch_ids += 1
#         if len(inputs_v)>0:
#             corpus.append(inputs_v)
#     return corpus, batch_ends_ids


def recollect_data(corpus, seq_lengths, source_len):
    batch_size = corpus[0][0].shape[0]
    assert batch_size % source_len == 0, "Batch size is not denominator of given data list length"
    dim_size = corpus[0][0].shape[1]
    sub_batch_size = batch_size/source_len
    
    data_list = []
    b_idx = 0
    for source_id in xrange(source_len):
        cuml = 0
        for sub_bi in xrange(sub_batch_size):
            cuml += seq_lengths[b_idx]
            b_idx += 1
        data_list.append(np.zeros((dim_size, cuml)))

    source_ids = np.zeros(source_len, dtype = np.int32)
    source_ends_ids = np.asarray([ d.shape[1] for d in data_list ])
    
    batch_ids = np.zeros(batch_size, dtype = np.int32) 
    
    for c in corpus:
        for seq_id, seq in enumerate(c):
            sub_batch_id = 0
            cuml_id = 0
            for b_num, b_id in enumerate(batch_ids):
                source_id = b_num / sub_batch_size
                dst_id = cuml_id + b_id
                # data_list[source_id][]
                # print c_id, seq_id, b_num, b_id
                if b_id < seq_lengths[b_num]: # source_ends_ids[source_id]:
                    # print source_id, cuml_id, b_id, " -> ", dst_id, " ends in ", source_ends_ids[source_id] 
                    data_list[source_id][:, dst_id] = seq[b_num, :]
                # else:
                #     print "Source ", source_id, " ended with ", dst_id
                
                sub_batch_id += 1
                if sub_batch_id < sub_batch_size:
                    cuml_id += seq_lengths[b_num]
                else:
                    cuml_id = 0
                    sub_batch_id = 0
                
                
            batch_ids += 1
    return data_list
