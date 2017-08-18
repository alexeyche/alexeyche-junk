
import tensorflow as tf

from tensorflow.contrib.rnn import RNNCell as RNNCell

class LCACell(RNNCell):
    def __init__(self, input_size, layer_size, filter_len, c):
        self._layer_size = layer_size
        self._filter_len = filter_len
        self._input_size = input_size
        self._c = c
        self._params = None

    @property
    def layer_size(self):
        return self._layer_size
    
    @property
    def filter_len(self):
        return self._filter_len
    
    @property
    def input_size(self):
        return self._input_size

    @property
    def state_size(self):
        return (self._layer_size, self._layer_size, self._layer_size, self._layer_size)

    @property
    def output_size(self):
        return (self._layer_size, self._layer_size, self._filter_len * self._input_size)

    def _init_parameters(self):
        c = self._c
        return (
            tf.get_variable("F", [self._filter_len * self._input_size, self._layer_size], 
                initializer=tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
            ),
        )

    def __call__(self, input, state, scope=None):
        with tf.variable_scope(scope or type(self).__name__):
            ### init
            if self._params is None:
                self._params = self._init_parameters()

            x = input[0]
            c = self._c
            
            batch_size, filter_len, input_size = x.get_shape().as_list()
            
            u, a, a_m, dF = state
            F = self._params[0]
            
            Fc = tf.matmul(tf.transpose(F), F) - tf.eye(self._layer_size)

            x_flat = tf.reshape(x, (batch_size, filter_len * input_size))

            #### logic
                        
            du = - u + tf.matmul(x_flat, F) - tf.matmul(a, Fc)
            
            new_u = u + c.epsilon * du / c.tau
            
            # threshold = a_m
            threshold = c.lam
            
            new_a = tf.nn.relu(new_u - threshold)
            new_a_m = (1.0 - 1.0/c.tau_m) * a_m + (1.0/c.tau_m) * new_a
            
            #### learning
            
            if c.simple_hebb:
                new_dF = dF + c.grad_accum_rate * (
                    tf.matmul(tf.transpose(x_flat), new_a)
                )
            else:
                new_dF = dF + c.grad_accum_rate * (
                    tf.matmul(tf.transpose(x_flat), new_a) 
                    - tf.matmul(F, tf.matmul(tf.transpose(new_a), new_a))
                )

            
            x_hat_flat = tf.matmul(new_a, tf.transpose(F))
            
            return (new_u, new_a, x_hat_flat), (new_u, new_a, new_a_m, new_dF)

    @property
    def F(self):
        assert self._params is not None
        return tf.reshape(self._params[0], (self._filter_len, self._input_size, self._layer_size))

    @property
    def F_flat(self):
        assert self._params is not None
        return self._params[0]


def normalize_weights(net):
    ops = []
    for cell in net._cells:
        F = cell._params[0]
        if cell._c.simple_hebb:
            ops.append(tf.assign(F, tf.nn.l2_normalize(F, 0)))
        else:
            ops.append(tf.identity(F))
            # ops.append(tf.assign(F, tf.nn.l2_normalize(F, 0)))
    return tf.group(*ops)
