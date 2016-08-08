import numpy as np

import tensorflow as tf
from tensorflow.python.ops import variable_scope as vs
import logging
from env import current as env
import cPickle as pkl
from util import norm
import librosa as lr
import os
import scipy

def generate_gammatone_dictionary(l, size, low_f=10, high_f=1000):
    def gammatone(t, f, phase, ampl, n, b):
        return ampl * t ** (n-1) * np.exp(-2.0 * np.pi * b * t) * np.cos(2.0 * np.pi * f * t + phase)
    gammatone_filters = np.zeros((l, size))

    t = np.linspace(0, 0.05, l)
    freqs = np.linspace(low_f, high_f, size)

    for fi, freq in enumerate(freqs):
        gammatone_filters[:, fi] = gammatone(t, freq, 0.0, 1.0, 2.0, 20.0)

    return gammatone_filters/np.sqrt(np.sum(gammatone_filters ** 2, 0))


def generate_dct_dictionary(l, size):
    p = np.asarray(xrange(l))
    filters = np.zeros((l, size))
    for fi in xrange(size):
        filters[:, fi] = np.cos((np.pi * (2 * fi + 1) * p)/(2*l))
        filters[0, fi] *= 1.0/np.sqrt(2.0)
        # filters[fi, 1:] *= np.sqrt(2/l) 
    return filters * np.sqrt(2.0/l)



class ConvConfig(object):
    def __init__(self, **kwargs):
        self.batch_size = kwargs.get("batch_size", 30000)
        self.filter_len = kwargs.get("filter_len", 150)
        self.filters_num = kwargs.get("filters_num", 100)
        self.target_sr = kwargs.get("target_sr", 3000)
        self.gamma = kwargs.get("gamma", 1e-03)
        self.strides = kwargs.get("strides", 8)
        self.avg_window = kwargs.get("avg_window", 5)
        self.lrate = kwargs.get("lrate", 1e-04)

class ConvDatasetInfo(object):
    def __init__(self):
        self.read_songs_info = []


def save_as_sparse(dense, dst_ptr):
    sparse_data = scipy.sparse.csr_matrix(dense)
    np.savez(dst_ptr, data = sparse_data.data ,indices=sparse_data.indices, indptr =sparse_data.indptr, shape=sparse_data.shape)


def load_sparse(src_ptr):
    loader = np.load(src_ptr)
    return scipy.sparse.csr_matrix((loader['data'], loader['indices'], loader['indptr']), shape = loader['shape'])



class ConvModel(object):
    MODEL_FNAME = "conv_model.ckpt"
    RECOV_FILTER_FNAME = "recov_filter.pkl"
    CONV_STATE = "conv_state.pkl"



    def __init__(self, *args, **kwargs):
        if len(args) == 2:
            assert isinstance(args[0], ConvConfig), "Need first arg as config"
            assert isinstance(args[1], ConvDatasetInfo), "Need second arg as dataset info"
            assert len(kwargs) == 0, "Need kwargs or two arguments with config and dataset info"

            self.cfg = args[0]
            self.ds_info = args[1]
        else:  
            assert len(args) == 0, "Set config parameters in kwargs"
            self.cfg = ConvConfig(**kwargs)
            self.ds_info = ConvDatasetInfo()

        self.build()
        
    def _read_song(self, fname, proportion=None):
        logging.info("Reading {}".format(fname))    
        song_data_raw, source_sr = lr.load(fname)
        logging.info("Got sampling rate {}, resampling to {} ...".format(source_sr, self.cfg.target_sr))
        song_data = lr.resample(song_data_raw, source_sr, self.cfg.target_sr, scale=True)
        logging.info("Normalizing with l2 norm ...")
        if proportion:
            song_data = song_data[:int(proportion*len(song_data)),]
        song_data, data_denom = norm(song_data)
        logging.info("Done")
        return song_data, source_sr, data_denom


    def form_dataset(self, files, proportion=None):
        self.ds_info = ConvDatasetInfo()
        dataset = []
        for source_id, fname in enumerate(files):
            song_data, source_sr, data_denom = self._read_song(fname, proportion)
            dataset.append(song_data)
            self.ds_info.read_songs_info.append((source_id, source_sr, data_denom))
        return dataset
    
    def get_data_info(self, source_id):
        assert source_id < len(self.ds_info.read_songs_info), "Can't find any dataset information for source with id {}".format(source_id)
        return self.ds_info.read_songs_info[source_id][1], self.ds_info.read_songs_info[source_id][2]

    def build(self):
        c = self.cfg
        self.input = tf.placeholder(tf.float32, shape=(1, c.batch_size, 1, 1), name="Input")

        init = lambda shape, dtype: generate_dct_dictionary(
            c.filter_len, 
            c.filters_num
        ).reshape(c.filter_len, 1, 1, c.filters_num)
        
        # init = lambda shape, dtype: np.reshape(-np.sqrt(3) / np.sqrt(L) + np.random.random((L, filters_num)) * 2.0*np.sqrt(3) / np.sqrt(L), (L, 1, 1, filters_num))
        # init = lambda shape, dtype: generate_gammatone_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)


        self.filter = vs.get_variable("W", [c.filter_len, 1, 1, c.filters_num], initializer=init)            
        self.bias = vs.get_variable("b", [c.filters_num], initializer=lambda shape, dtype: np.zeros(c.filters_num))
        self.recov_filter = vs.get_variable("Wr", [c.filter_len, 1, 1, c.filters_num], initializer=init)

        hidden = tf.nn.conv2d(self.input, self.filter, strides=[1, c.strides, 1, 1], padding='VALID')
        self.hidden_t = tf.pow(tf.nn.relu(hidden), 2.0)
        # hidden_t = tf.nn.relu(hidden, bias)

        self.hidden_p = tf.nn.avg_pool(self.hidden_t, [1, c.avg_window, 1, 1], strides = [1,1,1,1], padding='SAME')
        # hidden_p = hidden_t
        # hidden_t = tf.pow(hidden, 2.0)

        # hidden_t = tf.abs(hidden)
        self.hidden_p = self.hidden_p/tf.maximum(tf.reduce_max(self.hidden_p, [1]), tf.constant(1e-10))
        # hidden_p = tf.nn.l2_normalize(hidden_p, dim=1)

        self.output = tf.nn.conv2d_transpose(self.hidden_p, self.recov_filter, output_shape = (1, c.batch_size, 1, 1), strides=[1, c.strides, 1, 1], padding='VALID')
        self.output = tf.nn.l2_normalize(self.output, dim=1)


        self.cost = tf.nn.l2_loss(self.output - self.input) + c.gamma * tf.reduce_mean(self.hidden_t)  #+ 0.01 * gamma * tf.reduce_sum(hidden_t)

        self.optimizer = tf.train.AdamOptimizer(c.lrate)
        # self.optimizer = tf.train.RMSPropOptimizer(lrate)
        tvars = tf.trainable_variables()
        grads = tf.gradients(self.cost, tvars)
        self.apply_grads = self.optimizer.apply_gradients(zip(grads, tvars))


    def train_step(self, sess, data):
        out_v, h_v_raw, h_v, filter_v, rfilter_v, bias_v, cost_v, _ = sess.run(
            [
                self.output, 
                self.hidden_t, 
                self.hidden_p, 
                self.filter, 
                self.recov_filter, 
                self.bias, 
                self.cost, 
                self.apply_grads
            ], 
            {
                self.input: data.reshape(1, self.cfg.batch_size, 1, 1)
            }
        )
        h_v_raw = h_v_raw.reshape(h_v_raw.shape[1], h_v_raw.shape[3])
        h_v = h_v.reshape(h_v.shape[1], h_v.shape[3])
        return out_v, h_v, filter_v, rfilter_v, bias_v, cost_v

    def roll_around(self, sess, data, collect_output=False):
        mc = []
        
        output_data = []
        hidden_data = []
        zero_hidden = set(xrange(self.cfg.filters_num))
        for id_start in xrange(0, data.shape[0], self.cfg.batch_size):
            data_slice = data[id_start:min(id_start+self.cfg.batch_size, data.shape[0])]
            
            batch_data = np.zeros(self.cfg.batch_size)
            batch_data[:len(data_slice)] = data_slice
            out_v, h_v, filter_v, rfilter_v, bias_v, cost_v = self.train_step(sess, batch_data)
            
            # plt.figure(1)
            # plt.subplot(2,1,1)
            # plt.plot(h_v_raw[0:2000,21])
            # plt.subplot(2,1,2)
            # plt.plot(h_v[0:2000,21])
            # plt.show()
            zero_hidden &= set(np.where(np.mean(h_v, 0) == 0.0)[0])

            if collect_output:
                hidden_data.append(h_v)
                output_data.append(out_v.reshape(self.cfg.batch_size))
            mc.append(np.mean(cost_v))
        mean_cost = sum(mc)/len(mc)
        sparsity = float(len(zero_hidden))/self.cfg.filters_num

        if collect_output:
            return mean_cost, sparsity, filter_v, rfilter_v, bias_v, np.concatenate(hidden_data), np.concatenate(output_data)
        return mean_cost, sparsity, filter_v, rfilter_v, bias_v

    def train(self, sess, dataset, epochs, need_init=True):    
        if need_init:
            sess.run(tf.initialize_all_variables())

        for e in xrange(epochs):
            mc, sp = [], []
            for data in dataset:
                mean_cost, sparsity, filter_v, rfilter_v, bias_v = self.roll_around(sess, data)
                
                mc.append(mean_cost)
                sp.append(sparsity) 

            logging.info("Epoch {}, cost {}, sparsity {}".format(e, sum(mc)/len(mc), sum(sp)/len(sp)))

    
    def evaluate_and_save(self, sess, dataset):
        for data_id, data in enumerate(dataset):
            mean_cost, sparsity, filter_v, rfilter_v, bias_v, hidden_final, _ = self.roll_around(sess, data, collect_output = True)
            
            dump_fname = env.dataset("{}_sparse_acoustic_data.pkl".format(data_id))
            logging.info("Saving hidden data in {}".format(dump_fname))
            save_as_sparse(hidden_final, open(dump_fname, "w"))
            
            out_final = self.restore_hidden(hidden_final, rfilter_v.reshape(self.cfg.filter_len, self.cfg.filters_num))
            
            source_sr, data_denom = self.get_data_info(data_id)

            out_final *= data_denom
            data_recov = lr.resample(out_final, self.cfg.target_sr, source_sr, scale=True)
            
            recov_fname = env.result("{}_recovery.wav".format(data_id))
            logging.info("Saving recovery as {}".format(recov_fname))
            lr.output.write_wav(recov_fname, data_recov, source_sr)



    def restore_hidden(self, hidden_data, recov_filter=None):
        if recov_filter is None:
            recov_filter_fname = env.run(ConvModel.RECOV_FILTER_FNAME)
            assert os.path.exists(recov_filter_fname), "Need recovery filter filename {} to recover".format(recov_filter_fname)
            recov_filter = np.load(open(recov_filter_fname))
            recov_filter = recov_filter.reshape(150, 100) # HACK TODO

        sess = tf.Session()

        output_data = []
        for id_start in xrange(0, hidden_data.shape[0], self.cfg.batch_size):
            data_slice = hidden_data[id_start:min(id_start+self.cfg.batch_size, hidden_data.shape[0])]
            data_slice[np.where(data_slice < 1e-02)] = 0.0
            
            hidden_input = tf.placeholder(tf.float32, shape=(1, data_slice.shape[0], 1, data_slice.shape[1]), name="Input")
            recov_filter_input = tf.placeholder(
                tf.float32, 
                shape=(recov_filter.shape[0], 1, 1, recov_filter.shape[1]), 
                name="RecovFilter"
            )
            
            output = tf.nn.conv2d_transpose(
                hidden_input, 
                recov_filter_input, 
                output_shape = (1, self.cfg.strides*data_slice.shape[0]+recov_filter.shape[0]-1, 1, 1), 
                strides=[1, self.cfg.strides, 1, 1], 
                padding='VALID'
            )
            output = tf.nn.l2_normalize(output, dim=1)
            out_v = sess.run([output], {
                recov_filter_input: recov_filter.reshape(recov_filter.shape[0], 1, 1, recov_filter.shape[1]), 
                hidden_input: data_slice.reshape(1, data_slice.shape[0], 1, data_slice.shape[1])
            })
            output_data.append(out_v[0].reshape(out_v[0].shape[1]))
        return np.concatenate(output_data)

    def serialize(self, sess):
        model_fname = env.run(ConvModel.MODEL_FNAME)
        logging.info("Saving model in {}".format(model_fname))
        saver = tf.train.Saver()
        saver.save(sess, model_fname)
        
        dst = open(env.run(ConvModel.CONV_STATE), "w")
        
        pkl.dump(self.cfg, dst)
        pkl.dump(self.ds_info, dst)
        recov_filter_fname = env.run(ConvModel.RECOV_FILTER_FNAME)
        np.save(
            open(recov_filter_fname, "w"), 
            sess.run(self.recov_filter).reshape(self.cfg.filter_len, self.cfg.filters_num)
        )

    @staticmethod
    def deserialize(sess=None):        
        src = open(env.run(ConvModel.CONV_STATE))
        cfg = pkl.load(src)
        ds_info = pkl.load(src)
        cm = ConvModel(cfg, ds_info)
        if sess:
            saver = tf.train.Saver()
            modelf = env.run(ConvModel.MODEL_FNAME)
            if os.path.exists(modelf):
                logging.info("Restoring from {}".format(modelf))
                saver.restore(sess, modelf)

        return cm



