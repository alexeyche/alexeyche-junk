
import time

from util import shl, shm, shs

from tensorflow.python.ops import rnn
# import tensorflow.contrib.rnn as rnn

import numpy as np
from sklearn.metrics import mean_squared_error as MSE, mean_absolute_error as MAE



from config import Config
from retf.model import *

import sys
sys.path.insert(0, "/home/alexeyche/distr/hyperband")
from common_defs import *


def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)

def smooth(signal, sigma=0.005, filter_size=50):
    lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
    return np.convolve(lf_filter, signal, mode="same")



batch_size = 1

T0 = 0.0
T = 400.0
dt = 1.0
num_steps = int(T/dt)
Tvec = np.linspace(T0, T, num_steps)


def get_params():
    params = sample({
        "tau_c": hp.uniform("tau_c", 1.0, 25.0),
        "tau_a": hp.uniform("tau_a", 1.0, 25.0),
        "tau_b": hp.uniform("tau_b", 1.0, 25.0),
        "tau_syn": hp.uniform("tau_syn", 1.0, 25.0),
        "tau_alpha": hp.uniform("tau_alpha", 1.0, 25.0),
        "theta": hp.uniform("theta", 0.1, 25.0),
        "weight_init": hp.uniform("weight_init", 0.01, 5.0),
        "per_epoch_shift": hp.uniform("per_epoch_shift", 0.1, 25.0),
        "lrate": hp.loguniform("lrate", np.log(1e-01), np.log(5.0)),
        "bias_koeff": hp.uniform("bias_koeff", 0.001, 1.0),
    })

    return handle_integers(params)


###
def try_params(n_iterations, params, return_data=False):
    print params

    c = Config()

    c.batch_size = batch_size
    c.lambda_max = 500.0/1000.0      # 1/ms

    c.weight_init_factor = params["weight_init"]
    c.dt = dt
    c.tau_c = params["tau_c"]
    c.tau_b = params["tau_b"]
    c.tau_a = params["tau_a"]
    c.tau_syn = params["tau_syn"]

    c.Ee = 8.0
    c.Ei = -8.0
    c.alpha = c.dt/params["tau_alpha"]
    c.lrate = params["lrate"]

    ## ts related

    c.theta = params["theta"]
    c.per_epoch_shift = params["per_epoch_shift"]


    tf.set_random_seed(10)
    np.random.seed(10)
    ### data

    input_size = 300
    hidden_size = 50
    output_size = 20    
    x_values = np.zeros((num_steps, output_size))
    for ti in xrange(0, num_steps, 5):
        x_values[ti, ti % output_size] = 1.0


    # input_values = np.zeros((num_steps, input_size))
    # for ti in xrange(0, num_steps, 5):
    #     input_values[ti, input_size - (ti % input_size)-1] = 1.0

    # input_values = x_values.copy()
    input_values = poisson(0.02*np.random.random((num_steps, input_size)), c.dt)


    ###
    tf.reset_default_graph()
    
    net = DnnNet(
        # DnnCell((input_size, hidden_size, output_size), act=tf.sigmoid, is_output_cell=False, c=c),
        DnnCell((input_size, output_size, output_size), act=tf.sigmoid, is_output_cell=True, c=c),
    )

    x = tf.placeholder(tf.float32, (None, num_steps, input_size), name="x")
    y = tf.placeholder(tf.float32, (None, num_steps, output_size), name="y")

    modulation = tf.placeholder(tf.float32, (None, num_steps, 1))

    state = tuple(
        tuple(
            tf.placeholder(tf.float32, (None,) + (size if isinstance(size, tuple) else (size,)))
            for size in cell.state_size
        ) 
        for cell in net.cells
    ) + (tf.placeholder(tf.float32, (None, output_size)),)


    out, finstate = rnn.dynamic_rnn(net, (x, y, modulation), initial_state=state, dtype=tf.float32)


    # x_list = tf.unstack(x, num_steps, 1)
    # y_list = tf.unstack(y, num_steps, 1)

    # modulation_list = tf.unstack(modulation, num_steps, 1)

    # out, finstate = rnn.static_rnn(net, zip(x_list, y_list, modulation_list), initial_state=state, dtype=tf.float32)

    optimizer = tf.train.AdamOptimizer(c.lrate)
    # optimizer = tf.train.GradientDescentOptimizer(20.0)

    grads_and_vars = []
    for li, s in enumerate(finstate[:-1]):
        grads_and_vars += [
            (tf.reduce_mean(s[5], 0)/num_steps, net.cells[li]._params[0]),
            (params["bias_koeff"] * tf.reduce_mean(s[6], 0)/num_steps, net.cells[li]._params[1]),
        ]

        # if li < len(net.cells)-1:
        #     grads_and_vars.append(
        #         (tf.reduce_mean(s[7], 0)/num_steps, net.cells[li]._params[2]),
        #     )
        

    apply_grads_step = optimizer.apply_gradients(grads_and_vars)



    init = tf.global_variables_initializer()

    ####################################################################

    get_zero_state = lambda: tuple(
        np.zeros((batch_size,) + tuple(t.get_shape().as_list()[1:])) 
        for tup in state[:-1] for t in tup
    ) + (
        np.zeros((batch_size, output_size)),
    )


    sess = tf.Session()
    sess.run(init)


    input_values_b = np.expand_dims(input_values, 0)
    x_values_sm = np.asarray([smooth(x_values[:,ni]) for ni in xrange(output_size)]).T
    x_values_sm_b = np.expand_dims(x_values_sm, 0)


    for e in xrange(int(n_iterations)):
        rhythm = np.sin(2.0 * np.pi * Tvec * c.theta/T + e * c.per_epoch_shift/(2.0*np.pi))
        rhythm = (rhythm + 1.0)/2.0

        rhythm_b = np.expand_dims(np.expand_dims(rhythm, 0), 2)
        
        out_v, state_v, _ = sess.run(
            (out, finstate, apply_grads_step,),
        {
            x: input_values_b,
            y: x_values_sm_b,
            modulation: rhythm_b,
            state: get_zero_state() 
        })


        out_v_test, state_v_test = sess.run(
            (out, finstate,),
        {
            x: input_values_b,
            y: x_values_sm_b,
            modulation: np.zeros((batch_size, num_steps, 1)),
            state: get_zero_state() 
        })
        rmse = np.sum(np.square(out_v_test[-1][0]/c.lambda_max - x_values_sm_b))

        # mse = MSE(np.squeeze(x_values_sm_b), np.squeeze(out_v_test[-1][0]/c.lambda_max))
        # rmse = np.sqrt(mse)
        
        mae = MAE(np.squeeze(x_values_sm_b), np.squeeze(out_v_test[-1][0]/c.lambda_max))
    sess.close()
    r = { 'loss': rmse, 'rmse': rmse, 'mae': mae}
    if return_data:
        r["data"] = (out_v, out_v_test)
    print rmse
    return r

from hyperband import Hyperband

hb = Hyperband(get_params, try_params)
results = hb.run(skip_last = False)


# p = {'weight_init': 2.246314076891554, 'tau_b': 24.007448833081085, 'tau_c': 1.3402075787402872, 'tau_a': 10.881136694144896, 'lrate': 0.5851586265640217, 'theta': 24.0809893295545, 'tau_syn': 0.19291385527484867, 'per_epoch_shift': 22.910523230821795}
# r = try_params(10, p)

# p = {'weight_init': 0.09642636619530962, 'tau_b': 1.9493176363660059, 'tau_c': 1.7340754569936037, 'tau_a': 0.462867947572219, 'lrate': 0.6594933794300799, 'theta': 14.677925945506452, 'tau_syn': 20.646859326121326, 'per_epoch_shift': 22.329439955821854}
# r = try_params(10, p)

top_five = sorted(results, key=lambda k: k["loss"])[0:5]

r = try_params(81, top_five[0]["params"], return_data=True)

out_v, out_v_test = r["data"]