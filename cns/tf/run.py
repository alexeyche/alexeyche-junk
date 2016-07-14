
from model import ThetaRNNCell, generate_data
from model import laplace_act, gauss_act, epsp_act
from tensorflow.python.ops import rnn

import tensorflow as tf
import numpy as np
import os
from matplotlib import pyplot as plt


sigma = 0.05
dt = 0.1
#alpha=0.25

input_size = 10
batch_size = 1
net_size = 1
epochs = 300
seq_size = 10

lrate = 10.0
decay_rate=1




thetaNeuron = ThetaRNNCell(net_size, dt, sigma, activation = epsp_act)

signal_form = thetaNeuron.get_signal_form(9)

inputs  = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input_{}".format(idx)) for idx in xrange(seq_size) ]
targets = [ tf.placeholder(tf.float32, shape=(batch_size, net_size), name="Target") for si in xrange(seq_size) ]

init_state = state = tf.placeholder(tf.float32, shape=(batch_size, net_size), name="State")

outputs, finstate = rnn.rnn(thetaNeuron, inputs, init_state)


loss = tf.add_n([ tf.nn.l2_loss(target - outputs) for output, target in zip(outputs, targets) ]) / seq_size / batch_size / net_size

lr = tf.Variable(0.0, trainable=False)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(loss, tvars)
#grads, _ = tf.clip_by_global_norm(tf.gradients(loss, tvars), 5.0)

optimizer = tf.train.GradientDescentOptimizer(lr)
train_step = optimizer.apply_gradients(zip(grads_raw, tvars))




inputs_v, targets_v = generate_data(input_size, net_size, seq_size, batch_size, signal_form)
init_state_v = np.zeros((batch_size, net_size))


sess = tf.Session()
writer = tf.train.SummaryWriter("{}/tf".format(os.environ["HOME"]), sess.graph)

weights, recc_weights, bias = [], [], []
outputs_info, states_info, winput_info = [], [], []
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
        fetch += [thetaNeuron.W, thetaNeuron.bias]
        fetch += grads_raw
        fetch += [thetaNeuron.states_info, thetaNeuron.weighted_input_info]

        out = sess.run(fetch, feed_dict)

        outputs_v, state_v = np.asarray(out[0]), out[1]
        loss_v, train_step_v = out[2], out[3]
        
        weights.append((out[4], out[6]))
        bias.append((out[5], out[7]))
        
        print ", train loss {}".format(loss_v)
        states_info.append(out[8])
        winput_info.append(out[9])
        outputs_info.append(outputs_v)
        # if e % 10 == 0 or e == epochs-1:
            
            # plt.figure(1)
            # plt.subplot(2,1,1)
            # plt.plot(outputs_v[:,0,0])
            # plt.subplot(2,1,2)
            # plt.plot(np.cos(np.asarray(states_info[0])[:,0,0]))
            # plt.show()



plt.figure(1)
plt.subplot(2,1,1)
plt.plot(np.cos(np.asarray(states_info[0])[:,0,0]))
plt.subplot(2,1,2)
plt.plot(outputs_info[0][:,0,0])
plt.show()

plt.figure(1)
plt.subplot(2,1,1)
plt.plot(np.cos(np.asarray(states_info[-1])[:,0,0]))
plt.subplot(2,1,2)
plt.plot(outputs_info[-1][:,0,0])
plt.show()


w = np.asarray(weights)
b = np.asarray(bias)

plt.figure(1)
plt.subplot(2,1,1)
plt.imshow(w[:,0,:,0].T); 
plt.subplot(2,1,2)
plt.imshow(w[:,1,:,0].T); 
plt.show()


