
from model import ThetaRNNCell
from model import gauss_act, epsp_act, simple_act
from model import gen_poisson, dispatch_array, recollect_array

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

device = "gpu"

dt = 0.25
seed = 4
#alpha=0.25

input_size = 25
batch_size = 5
net_size = 10
epochs = 1000
bptt_steps = 50
seq_size = 1000
le_size = 2
lrate = 0.005
decay_rate = 1.0

neuron_in = ThetaRNNCell(input_size, dt, activation = simple_act, update_gate=True)
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


train_data = np.load(pj(os.environ["HOME"], "Music", "ml", "test_licks.data.npy"))
input_size = train_data.shape[0]
corpus = dispatch_array(train_data, bptt_steps, batch_size)

sfn=9
sf_sigma = 0.1
sf = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, sfn))/sf_sigma)
data = gen_poisson(np.asarray(input_size*[1.0/seq_size]), seq_size, 1.0, sf, seed)
corpus = dispatch_array(data.T, bptt_steps, batch_size)
# corpus = [[]]
# for seq_i in xrange(data.shape[0]):
#     corpus[-1].append(np.asarray([data[seq_i, :]]))
#     if len(corpus[-1]) % bptt_steps == 0:
#         corpus.append([])
# if len(corpus[-1]) == 0:
#     corpus = corpus[:-1]


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
        for corp_i, corp_data in enumerate(corpus):
            feed_dict = {k: v for k, v in zip(inputs, corp_data)}
            feed_dict[init_state] = state_v
            feed_dict.update({k: v for k, v in zip(targets, corp_data)})

            fetch = [outputs, finstate, loss, train_step]
            out = sess.run(fetch, feed_dict)

            outputs_v, state_v = np.asarray(out[0]), out[1]
            loss_v, _ = out[2], out[3]
            loss_sum += loss_v
            # states_info.append(out[4:])

            # outputs_info.append(outputs_v)
            # grads_info.append(out[4:])
        print "Epoch {}, learning rate {}".format(e, ep_lrate), "train loss {}".format(loss_sum/len(corpus))



    corpus_out, states = [], []
    for corp_i, corp_data in enumerate(corpus):
        feed_dict = {k: v for k, v in zip(inputs, corp_data)}
        feed_dict[init_state] = np.zeros((batch_size, state_size))
        feed_dict.update({k: v for k, v in zip(targets, corp_data)})

        fetch = [outputs, finstate]
        fetch += [neuron_in.W, neuron_in.U, neuron_in.W_u, neuron_in.U_u] 
        fetch += [neuron_le.W, neuron_le.U, neuron_le.W_u, neuron_le.U_u] 
        fetch += [neuron_out.W, neuron_out.U, neuron_out.W_u, neuron_out.U_u] 
        
        out = sess.run(fetch, feed_dict)
        outputs_v, state_v = out[0], out[1]
        params = out[2:]
        corpus_out.append(outputs_v)

    
plt.figure(1)
plt.subplot(2,1,1)
plt.imshow(recollect_array(corpus_out))
plt.subplot(2,1,2)
plt.imshow(recollect_array(corpus))
plt.show()



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
