
from model import ThetaRNNCell, generate_data, laplace_act

import tensorflow as tf
import numpy as np
import os
from matplotlib import pyplot as plt

sigma = 0.01
dt = 0.1


input_size = 100
batch_size = 1
net_size = 10
epochs = 1
seq_size = 100

lrate = 0.1
decay_rate=1




thetaNeuron = ThetaRNNCell(net_size, dt, sigma, activation = laplace_act)
signal_form = thetaNeuron.get_signal_form(9)

inputs  = [ tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input_{}".format(idx)) for idx in xrange(seq_size) ]
targets = [ tf.placeholder(tf.float32, shape=(batch_size, net_size), name="Target") for si in xrange(seq_size) ]

state = tf.placeholder(tf.float32, shape=(batch_size, net_size), name="State")
lr = tf.Variable(0.0, trainable=False)

# outputs, state = rnn.rnn(thetaNeuron, inputs, init_state)


sess = tf.Session()
writer = tf.train.SummaryWriter("{}/tf".format(os.environ["HOME"]), sess.graph)


#loss = tf.add_n([ tf.nn.l2_loss(target - outputs) for output, target in zip(outputs, targets) ]) / seq_size / batch_size / net_size

#lr = tf.Variable(0.0, trainable=False)
#tvars = tf.trainable_variables()
#grads_raw = tf.gradients(loss, tvars)
# grads, _ = tf.clip_by_global_norm(tf.gradients(loss, tvars), 5.0)

#optimizer = tf.train.GradientDescentOptimizer(lr)
#train_step = optimizer.apply_gradients(zip(grads_raw, tvars))



inputs_v, targets_v = generate_data(input_size, net_size, seq_size, batch_size, signal_form)
state_v = np.zeros((batch_size, net_size))
states, outs, wgrads, ugrads, bgrads = [], [], [], [], []
with tf.device("/cpu:0"):
    for idx, (inp, inp_v, targ, targ_v) in enumerate(zip(inputs, inputs_v, targets, targets_v)):
        print idx
        if idx > 0:
            tf.get_variable_scope().reuse_variables()
        sess.run(tf.assign(lr, lrate))

        out, new_state = thetaNeuron(inp, state)
        if idx == 0:
            init = tf.initialize_all_variables()
            sess.run(init)


        loss = tf.nn.l2_loss(targ - out)
        optimizer = tf.train.GradientDescentOptimizer(lr)

        tvars = tf.trainable_variables()
        grads_raw = tf.gradients(loss, tvars)
        train_step = optimizer.apply_gradients(zip(grads_raw, tvars))

        out_v, state_v, loss_v, grads_raw_v, _ = sess.run([out, new_state, loss, grads_raw, train_step], {inp: inp_v, state: state_v, targ: targ_v})
        states.append(state_v)
        outs.append(out_v)
        wgrads.append(grads_raw_v[0])
        ugrads.append(grads_raw_v[1])
        bgrads.append(grads_raw_v[2])

plt.imshow(np.asarray(wgrads)[:,:,0]); plt.show()
plt.imshow(np.asarray(ugrads)[:,:,0].T); plt.show()
plt.imshow(np.asarray(states)[:,0,:].T); plt.show()

