
from model import ThetaRNNCell
from model import gauss_act, epsp_act, simple_act

from tensorflow.python.ops import rnn

import tensorflow as tf
import numpy as np
import os
from matplotlib import pyplot as plt
from env import current as env
from os.path import join as pj
from glob import glob


def gen_poisson(rates, T, dt, signal_form=None, seed=None):
    if seed:
        np.random.seed(seed)
    res = dt * rates >  np.random.random((T, rates.shape[0]))
    res = res.astype(np.float32)
    if signal_form is not None:
        for ni in xrange(len(rates)):
            res[:, ni] = np.convolve(signal_form, res[:, ni], mode="same")
            res[:, ni] = np.clip(res[:, ni], 0.0, 1.0)
    return res

def sigmoid(x):
    return 1.0/(1.0 +np.exp(-x))



class Config(object):
    def __init__(self, **kwargs):
        self.seq_size = kwargs.get("seq_size", 100)
        self.batch_size = kwargs.get("batch_size", 1)
        self.lrate = kwargs.get("lrate", 1e-02)
        self.net_size = kwargs.get("net_size", 5)
        self.le_size = kwargs.get("le_size", None)
        self.num_of_layers = kwargs.get("num_of_layers", 1)
        self.forecast_ms = kwargs.get("forecast_ms", 10)
        self.epochs = kwargs.get("epochs", 200)


c = Config()
        


cell_type = ThetaRNNCell

if c.le_size:
    cells = rc.MultiRNNCell([cell_type(c.net_size), cell_type(c.le_size), cell_type(c.net_size)])
    net_out_size = cells._cells[-1].state_size
else:
    if c.num_of_layers > 1:
        cells = rc.MultiRNNCell([cell_type(c.net_size) for _ in xrange(c.num_of_layers)])
        net_out_size = cells._cells[-1].state_size
    else:
        cells = cell_type(c.net_size)
        net_out_size = cells.state_size

# input = tf.placeholder(tf.float32, shape=(c.seq_size, c.batch_size, c.net_size), name="Input")
# target = tf.placeholder(tf.float32, shape=(c.seq_size, c.batch_size, c.net_size), name="Target")

inputs = [ tf.placeholder(tf.float32, shape=(c.batch_size, c.net_size), name="Input{}".format(ii)) for ii in xrange(c.seq_size) ]
targets = [ tf.placeholder(tf.float32, shape=(c.batch_size, c.net_size), name="Target{}".format(ii)) for ii in xrange(c.seq_size) ]


state = tf.placeholder(tf.float32, shape=(c.batch_size, cells.state_size), name="State")

outputs, finstate = rnn.rnn(cells, inputs, state)


# net_out, finstate = rnn.dynamic_rnn(cells, input, initial_state = state, time_major=True)
cost = tf.add_n([ tf.nn.l2_loss(target - output) for output, target in zip(outputs, targets) ]) / c.seq_size / c.batch_size

# cost = tf.nn.l2_loss(target - net_out) / c.seq_size / c.batch_size

optimizer = tf.train.AdamOptimizer(c.lrate)
# optimizer = tf.train.RMSPropOptimizer(c.lrate)
# optimizer = tf.train.AdagradOptimizer(c.lrate)
# optimizer = tf.train.GradientDescentOptimizer(c.lrate)

tvars = tf.trainable_variables()
grads_raw = tf.gradients(cost, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

apply_grads = optimizer.apply_gradients(zip(grads, tvars))


# def run(state_v, inputs_v, targets_v):
#     return sess.run(
#         [
#             outputs,
#             finstate, 
#             cost, 
#             apply_grads,
#             grads
#         ], 
#         {
#             input: inputs_v.reshape(c.seq_size, 1, c.net_size),
#             target: targets_v.reshape(c.seq_size, 1, c.net_size),
#             state: state_v,
#         }
#     )

def eval(state_v, inputs_v):
    feed_dict = {k: v for k, v in zip(inputs, [ inputs_v[si, :].reshape(1, c.net_size) for si in xrange(c.seq_size) ])}
    feed_dict[state] = state_v
    outputs_v, finstate_v = sess.run(
        [
            outputs,
            finstate,
        ], 
        feed_dict
    )
    return np.concatenate(outputs_v), finstate_v




sess = tf.Session()
saver = tf.train.Saver()

model_fname = env.run("test_model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    c.epochs = 0
else:
    sess.run(tf.initialize_all_variables())


[ os.remove(f) for f in glob("{}/*.png".format(env.run())) ]


data_len = 2*c.seq_size
seed = 1
sfn=9
sf_sigma = 0.1
sf = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, sfn))/sf_sigma)
# data = gen_poisson(np.asarray(c.net_size*[5.0/c.seq_size]), data_len, 1.0, sf, seed)

data = np.zeros((c.net_size, data_len))
ni = 0
for si in xrange(0, data.shape[1], 5):
    data[ni, si] = 1.0
    ni += 1
    if ni >= c.net_size:
        ni = 0

for ni in xrange(data.shape[0]):
    data[ni, :] = np.convolve(sf, data[ni, :], mode="same")
    data[ni, :] = np.clip(data[ni, :], 0.0, 1.0)

data = data.T

def generate(start_id, ms = 200):
    state_v = np.zeros((c.batch_size, cells.state_size))

    inputs_v, state_v = eval(state_v, data[start_id:(start_id+c.seq_size)])

    generated = [inputs_v.reshape(c.seq_size, c.net_size)]

    for step_id in xrange(ms/c.forecast_ms):
        inputs_v, state_v = eval(state_v, inputs_v)
        generated.append(inputs_v.reshape(c.seq_size, c.net_size)[-c.forecast_ms:,])

    return np.concatenate(generated)


for epoch in xrange(c.epochs):
    p_sub = sigmoid(-6.0+12*epoch/float(c.epochs))
    if c.forecast_ms > 0:
        start_ids = np.arange(0, data.shape[0]-c.seq_size-c.forecast_ms, c.forecast_ms)
    else:
        start_ids = np.arange(0, data.shape[0], c.seq_size)
    if len(start_ids) == 0:
        start_ids = [0]
    losses = []
    
    out_v = []
    states_info_arr = []
    update_info_arr = []
    state_v = np.zeros((c.batch_size, cells.state_size))
        
    for i, start_id in enumerate(start_ids):
        
        fstart_id = start_id + c.forecast_ms
        
        inputs_v = data[start_id:(start_id+c.seq_size)] 
        targets_v = data[fstart_id:(fstart_id+c.seq_size)]

        feed_dict = {k: v for k, v in zip(inputs, [ inputs_v[si,:].reshape(1, c.net_size) for si in xrange(c.seq_size) ])}
        feed_dict[state] = state_v
        feed_dict.update({k: v for k, v in zip(targets, [ targets_v[si,:].reshape(1, c.net_size) for si in xrange(c.seq_size) ])})


        # if c.forecast_ms > 0 and out_v and p_sub > np.random.random_sample():
        #     inputs_v = out_v[-1]
        fetch = [outputs, finstate, cost, apply_grads, cells.states_info, cells.update_info]
        ret = sess.run(fetch, feed_dict)

        # ret = run(state_v, inputs_v, targets_v)
        
        curr_out_v, state_v, cost_v, _, states_info, update_info = ret
        
        out_v.append(np.concatenate(curr_out_v)) #curr_out_v.reshape(c.seq_size, c.net_size))
        states_info_arr.append(np.concatenate(states_info))
        update_info_arr.append(np.concatenate(update_info))
        # plt.figure(1)
        
        losses.append(cost_v)
    
        if i in frozenset([0, len(start_ids)/2, len(start_ids)-1]):
            vars_to_plot = []
            # vars_to_plot += ret[-1]
            # vars_to_plot += [ret[3], ret[4]]

            var_names = []
            # var_names = [ v.name for v in tvars ] 
            # var_names += ["net_out", "conv_out"]
            
            vars_to_plot = [ np.squeeze(vp) for vp in vars_to_plot ]
            for vi, dv in [ (vi, dv) for vi, dv in enumerate(vars_to_plot) if len(dv.shape) == 2]:
                if dv.shape[0] < dv.shape[1]:
                    plt.imshow(dv)
                else:
                    plt.imshow(dv.T)
                plt.colorbar()
                plt.savefig(env.run("{}_{}_{}.png".format(var_names[vi].replace("/", "_").replace(":",""), start_id, epoch)))        
                plt.clf()

            for vi, dv in [ (vi, dv) for vi, dv in enumerate(vars_to_plot) if len(dv.shape) == 1]:
                plt.plot(dv)
                plt.savefig(env.run("{}_{}_{}.png".format(var_names[vi].replace("/", "_").replace(":",""), start_id, epoch)))        
                plt.clf()            
            
            plt.figure(1)
            plt.subplot(2,1,1)
            plt.imshow(out_v[-1].T)
            plt.colorbar()
            plt.subplot(2,1,2)
            plt.imshow(targets_v.T)
            plt.colorbar()
            plt.savefig(env.run("{}_{}_{}.png".format("output_target", start_id, epoch)))  
            plt.clf()

            plt.figure(1)
            plt.subplot(2,1,1)
            plt.imshow(-np.cos(states_info_arr[-1].T)/2.0 + 0.5)
            plt.colorbar()
            plt.subplot(2,1,2)
            plt.imshow(update_info_arr[-1].T)
            plt.colorbar()
            plt.savefig(env.run("{}_{}_{}.png".format("states_and_update", start_id, epoch)))  
            plt.clf()
            
    print "Epoch {}, loss {}".format(epoch, sum(losses)/len(losses))
    if epoch % 5 == 0 or epoch == c.epochs - 1:
        if c.forecast_ms > 0:
            gen_out = generate(0)
            td = data[:len(gen_out),]
        else:
            gen_out = np.concatenate(out_v)
            td = data

        plt.figure(1)
        plt.subplot(2,1,1)
        plt.imshow(gen_out[:3000,:].T)
        plt.subplot(2,1,2)
        plt.imshow(td[:3000,:].T)
        plt.savefig(env.run("{}_{}.png".format("generated", epoch)))
        plt.clf()

# print "Saving model {}".format(saver.save(sess, model_fname))

