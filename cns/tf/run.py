
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
import scipy
import scipy.sparse

from env import Env

from conv_lib import SparseAcoustic

device = "gpu"

dt = 0.25
seed = 4
#alpha=0.25

net_size = 50
epochs = 200
bptt_steps = 50
le_size = 10
lrate = 0.0001
decay_rate = 1.0 #0.999

forecast_step = 1

env = Env("piano")



source_data_file_list = []

for f in sorted(os.listdir(env.dataset())):
    if f.endswith("sparse_acoustic_data.dump"):
        print "Considering {} as input".format(f)
        source_data_file_list.append(env.dataset(f))


data_file_list = source_data_file_list[:]

max_t, input_size = 0, None

for source_id, inp_file in enumerate(data_file_list):
    print "Reading {}".format(inp_file)
    d = SparseAcoustic.deserialize(inp_file)
    max_t = max(d.data.shape[0], max_t)
    assert input_size is None or input_size == d.data.shape[1], "Got spikes with another neurons number"
    input_size = d.data.shape[1]


batch_size = len(data_file_list)*5

corpus = Corpus(input_size, len(data_file_list), batch_size, max_t, bptt_steps)

sfn=9
sf_sigma = 0.1
sf = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, sfn))/sf_sigma)

for inp_file in data_file_list:
    print "Feeding {}".format(inp_file)
    d = SparseAcoustic.deserialize(inp_file)
    print "Feeding corpus ..."
    corpus.enrich_with_source(d.data)

gc.collect()

neuron_in = ThetaRNNCell(net_size, dt, activation = simple_act, update_gate=True)
# neuron_le = ThetaRNNCell(le_size, dt, activation = simple_act, update_gate=True)
neuron_out = ThetaRNNCell(input_size, dt, activation = simple_act, update_gate=True)

# neuron_in = ThetaRNNCell(input_size, dt, activation = simple_act, sigma = sigma)

# test_cell = rnn_cell.GRUCell
# neuron_in = test_cell(net_size)
# neuron_le = test_cell(le_size)
# neuron_out = test_cell(input_size, activation = sigmoid)

neurons = rnn_cell.MultiRNNCell([neuron_in, neuron_out])


# neurons = rnn_cell.MultiRNNCell([neuron_in, neuron_le])
# neurons_out = rnn_cell.MultiRNNCell([neuron_le, neuron_in])

state_size = neurons.state_size

inputs  = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input{}".format(idx)) for idx in xrange(bptt_steps) ]
targets = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Target{}".format(idx)) for idx in xrange(bptt_steps) ]

init_state = state = tf.placeholder(tf.float32, shape=(batch_size, state_size), name="State")

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

# official seq2seq (perfect regression)

outputs, finstate = ss.basic_rnn_seq2seq(inputs, targets, neurons)

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

sess = tf.Session()
saver = tf.train.Saver()

writer = tf.train.SummaryWriter("{}/tf".format(os.environ["HOME"]), sess.graph)

model_fname = env.run("nn_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
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
            corpus.prepare_sequence(corp_i, shift = forecast_step)
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

        plt.imshow(corpus.recollect(0, head=15000)[10000:15000, :].T)
        plt.subplot(2,1,2)
        plt.imshow(corpus_out.recollect(0, head=15000)[10000:15000, :].T)
        plt.savefig(env.run("{}_result.png".format(e)))
    
    gc.collect()
    
    print "Epoch {}, learning rate {}".format(e, ep_lrate), "train loss {}".format(loss_sum/corpus.shape[0])

if epochs>0:
    for source_id, f in enumerate(source_data_file_list):
        try:
            id = data_file_list.index(f)
        except ValueError:
            continue
        print "Recovering {}".format(source_id)
        d = SparseAcoustic.deserialize(f)
        d.data = scipy.sparse.csr_matrix(corpus_out.recollect(id))
        d.serialize(env.run("{}_nn_recovery.dump".format(source_id)))
            
    print "Saving model {}".format(saver.save(sess, model_fname))

# state_v = np.zeros((batch_size, state_size))
# corpus_out = Corpus.construct_from(corpus)
# loss_sum = 0
# for corp_i in xrange(corpus.shape[0]):
#     feed_dict = {k: v for k, v in zip(inputs, corpus.prepare_sequence(corp_i)) }
#     feed_dict[init_state] = state_v
#     feed_dict.update({k: v for k, v in zip(
#         targets, 
#         corpus.prepare_sequence(corp_i, shift = forecast_step)
#     )})


#     fetch = [outputs, finstate, loss]
#     fetch += [neuron_in.W, neuron_in.U, neuron_in.W_u, neuron_in.U_u] 
#     # fetch += [neuron_le.W, neuron_le.U, neuron_le.W_u, neuron_le.U_u] 
#     fetch += [neuron_out.W, neuron_out.U, neuron_out.W_u, neuron_out.U_u] 
    
#     out = sess.run(fetch, feed_dict)
#     outputs_v, state_v, loss_v = out[0], out[1], out[2]
#     params = out[3:]
    
#     loss_sum += loss_v

#     corpus_out.feed_corpus(corp_i, outputs_v)

# loss_sum = loss_sum/corpus.shape[0]

#     # le_corpus_out.feed_corpus(corp_i, states_v)


# state_v = np.zeros((batch_size, state_size))
# corpus_out = Corpus.construct_from(corpus)

# corpus_id = target_corpus_id = 0

# seq_id = 0
# test_seq = corpus.prepare_sequence(corpus_id)
# test_target_seq = corpus.prepare_sequence(target_corpus_id, shift = forecast_step)

# test_input_v = test_seq.pop(0)
# test_target_v = test_target_seq.pop(0)

# warming_up_steps = 125 # corpus.shape[0]*corpus.shape[1]


# for step_i in xrange(corpus.shape[0]*corpus.shape[1]):
#     feed_dict = {
#         test_input: test_input_v,
#         state: state_v,
#         test_target: test_target_v
#     }
#     test_output_v, state_v, test_loss_v = sess.run([test_output, test_finstate, test_loss], feed_dict)
#     print "Feeding {} {} {}".format(step_i / bptt_steps, seq_id, np.linalg.norm(test_output_v))
#     corpus_out.feed_batch(step_i / bptt_steps, seq_id, test_output_v)
    
#     if step_i < warming_up_steps:
#         test_input_v = test_seq.pop(0)
#         if len(test_seq) == 0:
#             corpus_id += 1
#             if corpus_id >= corpus.shape[0]:
#                 break
#             test_seq = corpus.prepare_sequence(corpus_id)
#     else:
#         test_input_v = test_output_v


#     test_target_v = test_target_seq.pop(0)
#     if len(test_target_seq) == 0:
#         target_corpus_id += 1
#         if target_corpus_id >= corpus.shape[0]:
#             break
#         test_target_seq = corpus.prepare_sequence(target_corpus_id, shift = forecast_step)
    
#     seq_id += 1
#     if seq_id >= bptt_steps:
#         seq_id = 0
#     print "Testing step {}, loss {}".format(step_i, test_loss_v)

# plt.figure(1)
# plt.subplot(2,1,1)
# # plt.imshow(corpus.data[0][49].todense())
# plt.imshow(corpus.recollect(0, head=25000)[10000:25000, :].T)
# plt.subplot(2,1,2)
# # plt.imshow(corpus_out.data[0][49].todense())
# plt.imshow(corpus_out.recollect(0, head=25000)[10000:25000, :].T)
# plt.show()
# for source_id, f in enumerate(source_data_file_list):
#     try:
#         id = data_file_list.index(f)
#     except ValueError:
#         continue
#     print "Saving dream {}".format(source_id)
#     d = SparseAcoustic.deserialize(f)
#     d.data = scipy.sparse.csr_matrix(corpus_out.recollect(id))
#     d.serialize(env.run("{}_nn_dream.dump".format(source_id)))
        
    
#     plt.figure(1)
#     plt.subplot(3,1,1)
#     plt.imshow(test_input_v)
#     plt.subplot(3,1,2)
#     plt.imshow(test_target_v)
#     plt.subplot(3,1,3)
#     plt.imshow(test_seq[0])
#     plt.show()


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
