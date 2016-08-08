
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
from conv_model import restore_hidden_from_env

device = "gpu"

dt = 0.25
seed = 4
#alpha=0.25

net_size = 50
epochs = 10000
bptt_steps = seq_size = 50
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

data_denoms = []
data_corpus = None
data_ends = []
for source_id, inp_file in enumerate(data_file_list):
    print "Reading {}".format(inp_file)
    d = SparseAcoustic.deserialize(inp_file)
    max_t = max(d.data.shape[0], max_t)
    assert input_size is None or input_size == d.data.shape[1], "Got spikes with another neurons number"
    input_size = d.data.shape[1]
    if data_corpus is None: 
        data_corpus = d.data
    else:
        scipy.sparse.vstack([data_corpus, d.data])
    data_ends.append(data_corpus.shape[0])
    data_denoms.append(d.data_denom)

batch_size = len(data_ends)*20
# data_ends = np.asarray([5000, data_ends[0]], dtype=np.int32)

def sigmoid(x): 
    return 1.0/(1.0 +np.exp(-x))

def get_random_batch_ids(data_ends, seq_size, batch_size, target_shift):
    data_len = len(data_ends)
    data_lens = np.diff(np.concatenate([np.asarray([0]), data_ends]))

    data_ids = np.random.choice(data_len, batch_size)
    batch_ids = np.asarray([ np.random.choice(dl - seq_size - target_shift) for dl in data_lens[data_ids] ])
    batch_ids += np.concatenate([np.asarray([0], dtype=np.int32), np.asarray(data_ends[:-1], dtype=np.int32)])[data_ids]
    return batch_ids


def get_sequence(data_corpus, batch_ids, target_shift, p, generated_input):
    inputs_v, targets_v = [], []
    batch_ids_work = batch_ids.copy()

    for seq_id in xrange(seq_size):
        inputs_v.append(data_corpus[batch_ids_work, :].todense())
        targets_v.append(data_corpus[batch_ids_work + target_shift, :].todense())
        batch_ids_work += 1
    if generated_input:
        if p == 1.0:
            inputs_v = generated_input
        else:
            for b_id in xrange(batch_size):
                if  p > np.random.random_sample():
                    # print "Patching {}".format(b_id)
                    for seq_id in xrange(seq_size):
                        inputs_v[seq_id][b_id, :] = generated_input[seq_id][b_id, :]

    return inputs_v, reversed(targets_v)


gc.collect()

neuron_in = ThetaRNNCell(net_size, dt, activation = simple_act, update_gate=True)
# neuron_le = ThetaRNNCell(le_size, dt, activation = simple_act, update_gate=True)
neuron_out = ThetaRNNCell(input_size, dt, activation = simple_act, update_gate=True)

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

_, enc_state = rnn.rnn(neurons, inputs, initial_state = state)
outputs, finstate = ss.rnn_decoder(targets, enc_state, neurons)

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

outputs_v = None
for e in xrange(epochs):
    state_v = np.zeros((batch_size, state_size))

    ep_lrate = lrate * (decay_rate ** e)
    sess.run(tf.assign(lr, ep_lrate))

    batch_ids = get_random_batch_ids(data_ends, seq_size, batch_size, forecast_step)
    inputs_v, targets_v = get_sequence(
        data_corpus,
        batch_ids, 
        forecast_step,
        sigmoid(-6.0+12*e/float(epochs)),
        outputs_v if outputs_v else None
    )
    feed_dict = {k: v for k, v in zip(inputs, inputs_v) }
    feed_dict[state] = state_v
    feed_dict.update({k: v for k, v in zip(targets, targets_v)})

    outputs_v, finstate_v, loss_v, _ = sess.run([outputs, finstate, loss, train_step], feed_dict)

    print "Epoch {}, learning rate {}".format(e, ep_lrate), "train loss {}".format(loss_v)

    # if e % 100 == 0:
    #     batch_ids = get_random_batch_ids(data_ends, 1001, batch_size, forecast_step)
    #     outputs_eval = []
    #     loss_eval = []
    #     for seq_id in xrange(1000):
    #         inputs_v, targets_v = get_sequence(
    #             data_corpus,
    #             batch_ids, 
    #             forecast_step,
    #             1.0,
    #             outputs_v
    #         )
    #         feed_dict = {k: v for k, v in zip(inputs, inputs_v) }
    #         feed_dict[state] = state_v
    #         feed_dict.update({k: v for k, v in zip(targets, targets_v)})

    #         outputs_v, state_v, loss_v, _ = sess.run([outputs, finstate, loss, train_step], feed_dict)
    #         outputs_eval.append(outputs_v)
    #         loss_eval.append(loss_v)
            
    #         batch_ids += 1

        # g = np.asarray(outputs_eval)
        # for b_id in xrange(batch_size):
        #     output = restore_hidden_from_env(sess, env, g[:,0, b_id,:], 8)
        #     output *= data_denoms[0]
        #     data_recov = lr.resample(output, 3000, 22000, scale=True)
        #     lr.output.write_wav(env.run("nn_dream_{}_{}.wav".format(e, b_id)), data_recov, 22000)

        print "Mean eval loss {}".format(sum(loss_eval)/float(len(loss_eval)))
        
    #     plt.figure(1)
    #     plt.subplot(2,1,1)

    #     plt.imshow(corpus.recollect(0, head=15000)[10000:15000, :].T)
    #     plt.subplot(2,1,2)
    #     plt.imshow(corpus_out.recollect(0, head=15000)[10000:15000, :].T)
    #     plt.savefig(env.run("{}_result.png".format(e)))
    
    gc.collect()
    


# test_inputs  = [ tf.placeholder(tf.float32, shape=(1, input_size), name="TestInput{}".format(idx)) for idx in xrange(bptt_steps) ]
# test_targets = [ tf.placeholder(tf.float32, shape=(1, input_size), name="TestTarget{}".format(idx)) for idx in xrange(bptt_steps) ]
# test_outputs, test_finstate = ss.basic_rnn_seq2seq(test_inputs, test_targets, neurons)
# test_loss = tf.add_n([ tf.nn.l2_loss(test_target - test_output) for test_output, test_target in zip(test_outputs, test_targets) ]) / bptt_steps / net_size


# inputs_v = []
# outputs_v = []
# generated = []
# for idx in xrange(10000):
#     targets_v = []
#     for seq_id in xrange(seq_size):
#         if idx == 0:
#             inputs_v.append(data_corpus[(idx+seq_id)].todense())
#         targets_v.append(data_corpus[(idx+seq_id + forecast_step)].todense())

#     feed_dict = {k: v for k, v in zip(test_inputs, inputs_v) }
#     feed_dict.update({k: v for k, v in zip(test_targets, targets_v)})

#     outputs_v, finstate_v, loss_v = sess.run([test_outputs, test_finstate, test_loss], feed_dict)
#     inputs_v = outputs_v
#     print "Test loss: {}".format(loss_v)
#     generated.append(outputs_v)

# g = np.asarray(generated).reshape(10000, 50, 100)
# sp = SparseAcoustic(g[:,0,:], data_denoms[0])
# sp.serialize(env.run("nn_dream.dump"))
        
# print "Saving model {}".format(saver.save(sess, model_fname))
    


# outputs_v = out[0], out[1]
# loss_v, _ = out[2], out[3]


    
# idx += 1



# if epochs>0:
#     for source_id, f in enumerate(source_data_file_list):
#         try:
#             id = data_file_list.index(f)
#         except ValueError:
#             continue
#         print "Recovering {}".format(source_id)
#         d = SparseAcoustic.deserialize(f)
#         d.data = scipy.sparse.csr_matrix(corpus_out.recollect(id))
#         d.serialize(env.run("{}_nn_recovery.dump".format(source_id)))
            
#     print "Saving model {}".format(saver.save(sess, model_fname))

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

# inputs_v = corpus.prepare_sequence(0)

# corpus_out = Corpus.construct_from(corpus)

# for corp_i in xrange(corpus.shape[0]):
#     feed_dict = {k: v for k, v in zip(inputs, inputs_v) }
#     feed_dict[init_state] = state_v
#     feed_dict.update({k: v for k, v in zip(
#         targets, 
#         corpus.prepare_sequence(corp_i, shift = forecast_step)
#     )})

#     fetch = [outputs, finstate, loss, train_step]
#     outputs_v, state_v, loss_v, _ = sess.run(fetch, feed_dict)
    
#     corpus_out.feed_batch(corp_i, outputs_v)
#     inputs_v = outputs_v
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
