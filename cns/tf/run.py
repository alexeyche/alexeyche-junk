
from model import ThetaRNNCell
from model import gauss_act, epsp_act, simple_act
from model import gen_poisson, Corpus

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

from mp_lib import SpikeRecords

device = "gpu"

dt = 0.25
seed = 4
#alpha=0.25

net_size = 100
epochs = 200
bptt_steps = 50
le_size = 10
lrate = 0.00001
decay_rate = 0.999

forecast_corpus_gap = 0

ds_dir = pj(os.environ["HOME"], "Music", "ml", "impro")
run_dir = pj(os.environ["HOME"], "Music", "ml", "impro_run")
if not os.path.exists(run_dir):
    os.makedirs(run_dir)



data_file_list = []

for f in sorted(os.listdir(ds_dir)):
    if f.endswith("spikes.pkl"):
        print "Considering {} as input".format(f)
        data_file_list.append(pj(ds_dir, f))


max_t, input_size = 0, None

for inp_file in data_file_list:
    print "Reading {}".format(inp_file)
    spike_object = pkl.load(open(inp_file, "rb"))
    max_t = max(max_t, max(spike_object.spike_times))
    assert input_size is None or input_size == spike_object.neurons_num, "Got spikes with another neurons number"
    input_size = spike_object.neurons_num


batch_size = len(data_file_list)*10


corpus = Corpus(input_size, len(data_file_list), batch_size, max_t, bptt_steps)

sfn=9
sf_sigma = 0.1
sf = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, sfn))/sf_sigma)

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


gc.collect()

neuron_in = ThetaRNNCell(net_size, dt, activation = simple_act, update_gate=True)
neuron_le = ThetaRNNCell(le_size, dt, activation = simple_act, update_gate=True)
neuron_out = ThetaRNNCell(input_size, dt, activation = simple_act, update_gate=True)

# neuron_in = ThetaRNNCell(input_size, dt, activation = simple_act, sigma = sigma)

# test_cell = rnn_cell.GRUCell
# neuron_in = test_cell(net_size)
# neuron_le = test_cell(le_size)
# neuron_out = test_cell(input_size, activation = sigmoid)

neurons = rnn_cell.MultiRNNCell([neuron_in, neuron_le, neuron_out])


# neurons = rnn_cell.MultiRNNCell([neuron_in, neuron_le])
# neurons_out = rnn_cell.MultiRNNCell([neuron_le, neuron_in])

state_size = neurons.state_size

inputs  = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input{}".format(idx)) for idx in xrange(bptt_steps) ]
targets = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Target{}".format(idx)) for idx in xrange(bptt_steps) ]

init_state = state = tf.placeholder(tf.float32, shape=(batch_size, state_size), name="State")

outputs, finstate = rnn.rnn(neurons, inputs, init_state)

# hand made seq2seq

# outputs_le, finstate = rnn.rnn(neurons, inputs, init_state)
# inp_state = array_ops.slice(finstate, [0, 0], [batch_size, input_size])
# le_state = array_ops.slice(finstate, [0, input_size], [batch_size, le_size])
# finstate = array_ops.concat(1, [le_state, inp_state])
# outputs, finstate = rnn.rnn(neurons_out, outputs_le, finstate, scope="out")

# official seq2seq (perfect regression)

# outputs, finstate = ss.basic_rnn_seq2seq(inputs, targets, neurons)

loss = tf.add_n([ tf.nn.l2_loss(target - output) for output, target in zip(outputs, targets) ]) / bptt_steps / batch_size / net_size

lr = tf.Variable(0.0, trainable=False)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(loss, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)


# optimizer = tf.train.GradientDescentOptimizer(lr)
# optimizer = tf.train.AdagradOptimizer(lr)
optimizer = tf.train.AdamOptimizer(lr)
# optimizer = tf.train.RMSPropOptimizer(lr)
# optimizer = tf.train.AdadeltaOptimizer(lr)

train_step = optimizer.apply_gradients(zip(grads, tvars))




weights, recc_weights, bias = [], [], []
outputs_info, states_info, winput_info = [], [], []
grads_info = []
with tf.device("/{}:0".format(device)):
    sess = tf.Session()
    writer = tf.train.SummaryWriter("{}/tf".format(os.environ["HOME"]), sess.graph)

    sess.run(tf.initialize_all_variables())
    for e in xrange(epochs):
        state_v = np.zeros((batch_size, state_size))

        ep_lrate = lrate * (decay_rate ** e)
        sess.run(tf.assign(lr, ep_lrate))
        loss_sum = 0
        corpus_out = Corpus.construct_from(corpus)
        for corp_i in xrange(corpus.shape[0]):
            feed_dict = {k: v for k, v in zip(inputs, corpus.prepare_sequence(corp_i)) }
            feed_dict[init_state] = state_v
            feed_dict.update({k: v for k, v in zip(
                targets, 
                corpus.prepare_sequence(corp_i + forecast_corpus_gap, allow_zeros=True)
            )})

            fetch = [outputs, finstate, loss, train_step]
            out = sess.run(fetch, feed_dict)

            outputs_v, state_v = out[0], out[1]
            loss_v, _ = out[2], out[3]
            loss_sum += loss_v
            # states_info.append(out[4:])

            # outputs_info.append(outputs_v)
            # grads_info.append(out[4:])
            corpus_out.feed_corpus(corp_i, outputs_v)

        if e % 5 == 0:
            plt.figure(1)
            plt.subplot(2,1,1)

            plt.imshow(corpus.recollect(0, head=2500))
            plt.subplot(2,1,2)
            plt.imshow(corpus_out.recollect(0, head=2500))
            plt.savefig(pj(run_dir, "{}_result.png".format(e)))
        
        gc.collect()
        
        print "Epoch {}, learning rate {}".format(e, ep_lrate), "train loss {}".format(loss_sum/corpus.shape[0])



    # corpus_out, states = [], []
    # for corp_i, corp_data in enumerate(corpus):
    #     feed_dict = {k: v for k, v in zip(inputs, corp_data)}
    #     feed_dict[init_state] = np.zeros((batch_size, state_size))
    #     feed_dict.update({k: v for k, v in zip(targets, corp_data)})

    #     fetch = [outputs, finstate]
    #     fetch += [neuron_le.states_info]
    #     fetch += [neuron_in.W, neuron_in.U, neuron_in.W_u, neuron_in.U_u] 
    #     fetch += [neuron_le.W, neuron_le.U, neuron_le.W_u, neuron_le.U_u] 
    #     fetch += [neuron_out.W, neuron_out.U, neuron_out.W_u, neuron_out.U_u] 
        
    #     out = sess.run(fetch, feed_dict)
    #     outputs_v, state_v, states_v = out[0], out[1], out[2]
    #     params = out[3:]
        
    #     corpus_out.append(outputs_v)
    #     states.append(states_v)

    



# plt.figure(1)
# plt.subplot(4,1,1)
# plt.imshow(np.asarray(inputs_v)[:,0,:].T)
# plt.subplot(4,1,2)
# plt.imshow(np.asarray(outputs_info)[0,:,0,:].T)
# plt.subplot(4,1,3)
# plt.imshow(np.asarray(outputs_info)[-1,:,0,:].T)
# plt.subplot(4,1,4)
# plt.imshow(np.asarray(targets_v)[:,0,:].T)
# plt.show()


# plt.plot(np.cos(np.asarray(states_info[-1])[:,0,:]))
# plt.show()
