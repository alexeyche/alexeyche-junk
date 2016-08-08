
from model import ThetaRNNCell

from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell
from tensorflow.python.ops import seq2seq as ss
from tensorflow.python.ops.math_ops import sigmoid
from tensorflow.python.ops import variable_scope
from tensorflow.python.ops import array_ops

import tensorflow as tf
import numpy as np
import os
from matplotlib import pyplot as plt
from os.path import join as pj
import sys
import cPickle as pkl
import gc
import scipy
import scipy.sparse
import librosa as lr

from env import current as env

from conv_model import ConvModel, load_sparse, save_as_sparse


def sigmoid(x): 
    return 1.0/(1.0 +np.exp(-x))

def get_random_batch_ids(data_ends, seq_size, batch_size, target_shift):
    data_len = len(data_ends)
    data_lens = np.diff(np.concatenate([np.asarray([0]), data_ends]))

    data_ids = np.random.choice(data_len, batch_size)
    batch_ids = np.asarray([ np.random.choice(dl - seq_size - target_shift) for dl in data_lens[data_ids] ])
    batch_ids += np.concatenate([np.asarray([0], dtype=np.int32), np.asarray(data_ends[:-1], dtype=np.int32)])[data_ids]
    return batch_ids


def get_sequence(data_corpus, batch_ids, seq_size, target_shift):
    inputs_v, targets_v = [], []
    
    for seq_id in xrange(seq_size):
        inputs_v.append(data_corpus[batch_ids, :].todense())
        targets_v.append(data_corpus[batch_ids + target_shift, :].todense())
        batch_ids += 1

    return inputs_v, targets_v





device = "gpu"

seed = 4
#alpha=0.25

net_size = 50
epochs = 200
bptt_steps = seq_size = 50
le_size = 10
lrate = 0.0001
decay_rate = 1.0 #0.999

forecast_step = 150
continuous_steps = 2

source_data_file_list = []

for f in sorted(os.listdir(env.dataset())):
    if f.endswith("sparse_acoustic_data.pkl"):
        print "Considering {} as input".format(f)
        source_data_file_list.append(env.dataset(f))


data_file_list = source_data_file_list[:]


max_t, input_size = 0, None


data_corpus = None
data_ends = []
for source_id, inp_file in enumerate(data_file_list):
    print "Reading {}".format(inp_file)
    d = load_sparse(inp_file)
    max_t = max(d.data.shape[0], max_t)
    assert input_size is None or input_size == d.shape[1], "Got spikes with another neurons number"
    input_size = d.shape[1]
    if data_corpus is None: 
        data_corpus = d
    else:
        scipy.sparse.vstack([data_corpus, d])
    data_ends.append(data_corpus.shape[0])
    
batch_size = len(data_ends)*20



gc.collect()

neuron_in = ThetaRNNCell(net_size)
# neuron_le = ThetaRNNCell(le_size, dt, activation = simple_act, update_gate=True)
neuron_out = ThetaRNNCell(input_size)

# neuron_in = ThetaRNNCell(input_size, dt, activation = simple_act, sigma = sigma)

# test_cell = rnn_cell.GRUCell
# neuron_in = test_cell(net_size)
# neuron_le = test_cell(le_size)
# neuron_out = test_cell(input_size, activation = sigmoid)

neurons = neuron_out #rnn_cell.MultiRNNCell([neuron_in, neuron_out])


# neurons = rnn_cell.MultiRNNCell([neuron_in, neuron_le])
# neurons_out = rnn_cell.MultiRNNCell([neuron_le, neuron_in])

state_size = neurons.state_size

inputs  = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input{}".format(idx)) for idx in xrange(bptt_steps) ]
targets = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Target{}".format(idx)) for idx in xrange(bptt_steps) ]

state = tf.placeholder(tf.float32, shape=(batch_size, state_size), name="State")

####################

# outputs, finstate = rnn.rnn(neurons, inputs, init_state)

####################

# hand made seq2seq

# outputs_le, finstate = rnn.rnn(neurons, inputs, init_state)
# inp_state = array_ops.slice(finstate, [0, 0], [batch_size, input_size])
# le_state = array_ops.slice(finstate, [0, input_size], [batch_size, le_size])
# finstate = array_ops.concat(1, [le_state, inp_state])
# outputs, finstate = rnn.rnn(neurons_out, outputs_le, finstate, scope="out")

####################

outputs, finstate = ss.rnn_decoder(inputs, state, neurons)

loss = tf.add_n([ tf.nn.l2_loss(target - output) for output, target in zip(outputs, targets) ]) / bptt_steps / batch_size / net_size

###

test_inputs  = [ tf.placeholder(tf.float32, shape=(1, input_size), name="TestInput{}".format(idx)) for idx in xrange(bptt_steps) ]
test_state = tf.placeholder(tf.float32, shape=(1, state_size), name="TestState")

test_outputs, test_finstate = ss.rnn_decoder(test_inputs, test_state, neurons)

###


lrate_var = tf.Variable(0.0, trainable=False)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(loss, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

optimizer = tf.train.AdamOptimizer(lrate_var)

train_step = optimizer.apply_gradients(zip(grads, tvars))

sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("nn_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.initialize_all_variables())







def generate():

    def zero_batch():
        return [ np.zeros((1, input_size)) for _ in xrange(bptt_steps) ]


    def start_batch():
        return [ data_corpus[seq_id, :].todense() for seq_id in xrange(bptt_steps) ]


    state_v = np.zeros((1, state_size))

    def run(inputs_v, state_v):
        feed_dict = {k: v for k, v in zip(test_inputs, inputs_v) }
        feed_dict[test_state] = state_v

        return sess.run([test_outputs, test_finstate], feed_dict)
        

    inputs_v, state_v = run(start_batch(), state_v)

    generated = []
    generated += inputs_v

    for gen_idx in xrange(100):
        inputs_v, state_v = run(inputs_v, state_v)
        generated += inputs_v

    return generated







for e in xrange(epochs):
    state_v = np.zeros((batch_size, state_size))

    ep_lrate = lrate * (decay_rate ** e)
    sess.run(tf.assign(lrate_var, ep_lrate))

    batch_ids = get_random_batch_ids(data_ends, continuous_steps*forecast_step, batch_size, forecast_step)
    
    p_sub = sigmoid(-6.0+12*e/float(epochs))
    
    losses = []
    seq_out = []

    for cont_epoch in xrange(continuous_steps):
        new_seq_out = []
        for step_id in xrange(0, forecast_step, bptt_steps):    
            inputs_v, targets_v = get_sequence(
                data_corpus,
                batch_ids,
                bptt_steps,
                forecast_step,
            )

            feed_dict = {k: v for k, v in zip(inputs, inputs_v) }
            feed_dict[state] = state_v
            feed_dict.update({k: v for k, v in zip(targets, targets_v)})

            if p_sub == 1.0 and len(seq_out)>0:
                inputs_v = seq_out[step_id:(step_id+bptt_steps)]
            elif len(seq_out)>0:
                for b_id in xrange(batch_size):
                    if  p_sub > np.random.random_sample():
                        for seq_id in xrange(bptt_steps):
                            inputs_v[seq_id][b_id, :] = seq_out[step_id + seq_id][b_id, :]


            outputs_v, state_v, loss_v, _ = sess.run([outputs, finstate, loss, train_step], feed_dict)
            new_seq_out += outputs_v
            losses.append(loss_v)
        
        seq_out = new_seq_out
        gc.collect()

    print "Epoch {}, learning rate {}".format(e, ep_lrate), "train loss {}".format(sum(losses)/float(len(losses)))
    if e % 100 == 0:
        print "Generating sample"
        generated = generate()
        generated = np.asarray(generated).reshape(len(generated), input_size)        
        cm = ConvModel.deserialize()
        waveform = cm.restore_hidden(generated)        
        source_sr, data_denom = cm.get_data_info(0)

        waveform *= data_denom
        resampled_waveform = lr.resample(waveform, cm.cfg.target_sr, source_sr, scale=True)
        
        gen_fname = env.run("{}_generated.wav".format(e))
        print "Saving generated as {}".format(gen_fname)
        lr.output.write_wav(gen_fname, resampled_waveform, source_sr)
