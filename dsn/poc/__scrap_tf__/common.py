from poc.util import *

from scipy.sparse import issparse
import tensorflow as tf

def relu_deriv(x):
    a = tf.nn.relu(x)
    return tf.gradients(a, [x])[0]


def sigmoid_deriv(x):
    v = tf.sigmoid(x)
    return v * (1.0 - v)


def dense_to_sparse(tensor):
    tensor = tf.convert_to_tensor(tensor)
    indices = tf.where(tf.not_equal(tensor, tf.constant(0, tensor.dtype)))
    values = tf.gather_nd(tensor, indices)
    shape = tf.shape(tensor, out_type=tf.int64)
    return tf.SparseTensor(indices, values, shape)

def flatten(p):
    return [pp for param in p for pp in param]

def init_parameters(
    input_size,
    net_structure,
    weight_factor,
    sparse,
    lil_epsilon=1.0,
    big_epsilon=10.0
):
    params, params_opt = [], []
    for fan_in, fan_out in zip((input_size,) + net_structure[:-1], net_structure):
        if sparse:
            epsilon = lil_epsilon if (float(fan_in) + fan_out) / (fan_in * fan_out) > 0.05 else big_epsilon
            Winit, binit = sparse_xavier_init(fan_in, fan_out, const=weight_factor, epsilon=epsilon)
            Winit = Winit.T

            Wflat = tf.Variable(Winit.data, name="W")
            b = tf.Variable(binit, name="b")
            W = tf.SparseTensor(np.asarray((Winit.row, Winit.col)).T, Wflat, (fan_out, fan_in))
            
            params_opt.append((Wflat, b))
            params.append((W, b))
        else:
            Winit, binit = xavier_init(fan_in, fan_out, const=weight_factor)
            W = tf.Variable(Winit.T, name="W")
            b = tf.Variable(binit, name="b")

            params.append((W, b))
    
    if sparse:
        return params, params_opt
    else:
        return params, params




class Network(object):
    def __init__(
        s,
        input_size,
        net_structure,
        weight_factor,
        net_act,
        net_act_deriv,
        output_act,
        output_act_deriv,
        loss_deriv,
        sparse=False,
        lil_epsilon=1.0,
        big_epsilon=10.0
    ):
        s.input_size = input_size
        s.net_structure = net_structure
        s.weight_factor = weight_factor

        s.net_act = net_act
        s.net_act_deriv = net_act_deriv

        s.output_act = output_act
        s.output_act_deriv = output_act_deriv
        s.loss_deriv = loss_deriv

        s.sparse = sparse

        s.lil_epsilon = lil_epsilon
        s.big_epsilon = big_epsilon



        s.p, s.po = init_parameters(
            input_size,
            net_structure,
            weight_factor,
            sparse,
            lil_epsilon,
            big_epsilon
        )

    
    def build_model(s, x, y):
        a = [None]*len(s.net_structure)
        u = [None]*len(s.net_structure)
        da = [None]*len(s.net_structure)
        

        for li, (W, b) in enumerate(s.p):
            inp = x if li == 0 else a[li - 1]
            f = s.net_act if li < len(s.net_structure)-1 else s.output_act

            if s.sparse:
                u[li] = tf.transpose(tf.sparse_tensor_dense_matmul(W, tf.transpose(inp))) + b
            else:
                u[li] = tf.transpose(tf.matmul(W, tf.transpose(inp))) + b
            
            a[li] = f(u[li])

        dp = []
        for li in reversed(range(len(s.net_structure))):
            act_deriv = s.net_act_deriv if li < len(s.net_structure) - 1 else s.output_act_deriv
            
            da[li] = (
                s.loss_deriv(a[-1], y) if li == len(s.net_structure) - 1 else
                (
                    tf.transpose(tf.sparse_tensor_dense_matmul(tf.sparse_transpose(s.p[li + 1][0]), tf.transpose(da[li + 1])))
                    if s.sparse else 
                    tf.matmul(da[li + 1], s.p[li + 1][0])
                )
            )

            layer_input = x if li == 0 else a[li - 1]
            
            dW = tf.transpose(tf.matmul(tf.transpose(layer_input), da[li] * act_deriv(u[li]))) 
            db = tf.reduce_sum(da[li], 0, name=str(li)) 
            if s.sparse:
                dW = tf.gather_nd(dW, s.p[li][0].indices)
            dp.append((dW, db))
        


        return a, u, da, list(reversed(dp))


    # METRIC_SIZE = 3
    # def calc_metrics(s, metrics, epoch, a, y, batches_num):
    #     metrics[epoch][0] += losa[-1], y) / batches_num
    #     metrics[epoch][1] += np.mean(np.not_equal(np.argmax(a[-1], 1), np.argmax(y, 1))) / batches_num
    #     if s.sparse:
    #         all_elem = a[0].shape[0] * a[0].shape[1]
    #         metrics[epoch][2] = (float(all_elem) - a[0].nnz)/ all_elem / batches_num
    #     else:
    #         metrics[epoch][2] += np.mean(np.equal(a[0], 0.0)) / batches_num

    # def train(s, ds, epochs, print_freq=10):
    
