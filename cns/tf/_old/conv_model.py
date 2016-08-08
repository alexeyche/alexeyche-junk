import numpy as np

import tensorflow as tf
from tensorflow.python.ops import variable_scope as vs
from conv_lib import norm

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


class ConvModel(object):
    def __init__(self, batch_size, L, filters_num, k, avg_size, lrate, gamma):

        self.input = tf.placeholder(tf.float32, shape=(1, batch_size, 1, 1), name="Input")

        init = lambda shape, dtype: generate_dct_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)
        # init = lambda shape, dtype: np.reshape(-np.sqrt(3) / np.sqrt(L) + np.random.random((L, filters_num)) * 2.0*np.sqrt(3) / np.sqrt(L), (L, 1, 1, filters_num))
        # init = lambda shape, dtype: generate_gammatone_dictionary(L, filters_num).reshape(L, 1, 1, filters_num)


        self.filter = vs.get_variable("W", [L, 1, 1, filters_num], initializer=init)            
        self.bias = vs.get_variable("b", [filters_num], initializer=lambda shape, dtype: np.zeros(filters_num))
        self.recov_filter = vs.get_variable("Wr", [L, 1, 1, filters_num], initializer=init)

        hidden = tf.nn.conv2d(self.input, self.filter, strides=[1, k, 1, 1], padding='VALID')
        self.hidden_t = tf.pow(tf.nn.relu(hidden), 2.0)
        # hidden_t = tf.nn.relu(hidden, bias)

        self.hidden_p = tf.nn.avg_pool(self.hidden_t, [1, avg_size, 1, 1], strides = [1,1,1,1], padding='SAME')
        # hidden_p = hidden_t
        # hidden_t = tf.pow(hidden, 2.0)

        # hidden_t = tf.abs(hidden)
        self.hidden_p = self.hidden_p/tf.maximum(tf.reduce_max(self.hidden_p, [1]), tf.constant(1e-10))
        # hidden_p = tf.nn.l2_normalize(hidden_p, dim=1)

        self.output = tf.nn.conv2d_transpose(self.hidden_p, self.recov_filter, output_shape = (1, batch_size, 1, 1), strides=[1, k, 1, 1], padding='VALID')
        self.output = tf.nn.l2_normalize(self.output, dim=1)


        self.cost = tf.nn.l2_loss(self.output - self.input) + gamma * tf.reduce_mean(self.hidden_t)  #+ 0.01 * gamma * tf.reduce_sum(hidden_t)

        optimizer = tf.train.AdamOptimizer(lrate)
        # optimizer = tf.train.RMSPropOptimizer(lrate)
        tvars = tf.trainable_variables()
        grads = tf.gradients(self.cost, tvars)
        self.train_step = optimizer.apply_gradients(zip(grads, tvars))
        self.batch_size = batch_size

    def run(self, sess, data):
        out_v, h_v_raw, h_v, filter_v, rfilter_v, bias_v, cost_v, _ = sess.run(
            [self.output, self.hidden_t, self.hidden_p, self.filter, self.recov_filter, self.bias, self.cost, self.train_step], {
                self.input: data.reshape(1, self.batch_size, 1, 1)
            }
        )
        h_v_raw = h_v_raw.reshape(h_v_raw.shape[1], h_v_raw.shape[3])
        h_v = h_v.reshape(h_v.shape[1], h_v.shape[3])
        return out_v, h_v, filter_v, rfilter_v, bias_v, cost_v


def restore_hidden(sess, recov_filter, hidden_data, k, batch_size=30000):
    output_data = []
    for id_start in xrange(0, hidden_data.shape[0], batch_size):
        data_slice = hidden_data[id_start:min(id_start+batch_size, hidden_data.shape[0])]
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
            output_shape = (1, k*data_slice.shape[0]+recov_filter.shape[0]-1, 1, 1), 
            strides=[1, k, 1, 1], 
            padding='VALID'
        )
        output = tf.nn.l2_normalize(output, dim=1)
        out_v = sess.run([output], {
            recov_filter_input: recov_filter.reshape(recov_filter.shape[0], 1, 1, recov_filter.shape[1]), 
            hidden_input: data_slice.reshape(1, data_slice.shape[0], 1, data_slice.shape[1])
        })
        output_data.append(out_v[0].reshape(out_v[0].shape[1]))
    return np.concatenate(output_data)


def restore_hidden_from_env(sess, env, hidden_data, k):
	recov_filter = np.load(env.run("recov_filter.pkl"))
	return restore_hidden(sess, recov_filter, hidden_data, k)

