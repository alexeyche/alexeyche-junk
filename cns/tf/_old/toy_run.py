
from model import ThetaRNNCell
from model import gauss_act, epsp_act, simple_act
from model import gen_poisson

from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell
from tensorflow.python.ops import seq2seq as ss
from tensorflow.python.ops.math_ops import sigmoid


import tensorflow as tf
import numpy as np
import os
from matplotlib import pyplot as plt


sigma = 0.5
dt = 0.01
seed = 4
#alpha=0.25

input_size = 100
batch_size = 1
net_size = 10
epochs = 1000
seq_size = 100
le_size = 2
lrate = 0.1
decay_rate=1



neuron_in = ThetaRNNCell(net_size, dt, activation = simple_act, sigma = sigma)
neuron_le = ThetaRNNCell(le_size, dt, activation = simple_act, sigma = sigma)
neuron_out = ThetaRNNCell(net_size, dt, activation = simple_act, sigma = sigma)

neurons = rnn_cell.MultiRNNCell([neuron_in, neuron_le, neuron_out])

# neuron = rnn_cell.BasicRNNCell(net_size, activation = sigmoid)
# neuron = rnn_cell.GRUCell(net_size, activation = sigmoid)

inputs  = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input_{}".format(idx)) for idx in xrange(seq_size) ]
targets = [ tf.placeholder(tf.float32, shape=(batch_size, net_size), name="Target") for si in xrange(seq_size) ]

init_state = state = tf.placeholder(tf.float32, shape=(batch_size, 2*net_size + le_size), name="State")


outputs, finstate = rnn.rnn(neurons, inputs, init_state)

loss = tf.add_n([ tf.nn.l2_loss(target - output) for output, target in zip(outputs, targets) ]) / seq_size / batch_size / net_size

lr = tf.Variable(0.0, trainable=False)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(loss, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

#optimizer = tf.train.GradientDescentOptimizer(lr)
optimizer = tf.train.AdagradOptimizer(lr)
train_step = optimizer.apply_gradients(zip(grads, tvars))

sfn=9
sf_sigma = 0.1
sf = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, sfn))/sf_sigma)
inputs_v = [ np.asarray([s]) for s in gen_poisson(np.asarray(input_size*[1.0/seq_size]), seq_size, 1.0, sf, seed) ]
targets_v = [ np.asarray([s]) for s in gen_poisson(np.asarray(net_size*[1.0/seq_size]), seq_size, 1.0, sf, seed) ]

init_state_v = np.zeros((batch_size, 2*net_size+le_size))



sess = tf.Session()
writer = tf.train.SummaryWriter("{}/tf".format(os.environ["HOME"]), sess.graph)

weights, recc_weights, bias = [], [], []
outputs_info, states_info, winput_info = [], [], []
grads_info = []
with tf.device("/cpu:0"):
    sess.run(tf.initialize_all_variables())
    for e in xrange(epochs):
        ep_lrate = lrate * (decay_rate ** e)
        print "Epoch {}, learning rate {}".format(e, ep_lrate),
        sess.run(tf.assign(lr, ep_lrate))

        feed_dict = {k: v for k, v in zip(inputs, inputs_v)}
        feed_dict[init_state] = init_state_v
        feed_dict.update({k: v for k, v in zip(targets, targets_v)})

        fetch = [outputs, finstate]
        fetch += [loss, train_step]
        # fetch += grads
        fetch += neuron_le.states_info

        out = sess.run(fetch, feed_dict)

        outputs_v, state_v = np.asarray(out[0]), out[1]
        loss_v, _ = out[2], out[3]
        states_info.append(out[4:])

        print ", train loss {}".format(loss_v)
        outputs_info.append(outputs_v)
        # grads_info.append(out[4:])



plt.figure(1)
plt.subplot(4,1,1)
plt.imshow(np.asarray(inputs_v)[:,0,:].T)
plt.subplot(4,1,2)
plt.imshow(np.asarray(outputs_info)[0,:,0,:].T)
plt.subplot(4,1,3)
plt.imshow(np.asarray(outputs_info)[-1,:,0,:].T)
plt.subplot(4,1,4)
plt.imshow(np.asarray(targets_v)[:,0,:].T)
plt.show()


plt.plot(np.cos(np.asarray(states_info[-1])[:,0,:]))
plt.show()
