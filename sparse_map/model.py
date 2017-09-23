
import tensorflow as tf

from tensorflow.contrib.rnn import RNNCell as RNNCell

def poisson(rate):
    return tf.cast(tf.less(tf.random_uniform(rate.get_shape()), rate), rate.dtype)

def exp_poisson(u, dt=0.001):
    return poisson(dt * tf.exp(u))
                

class LCACell(RNNCell):
    def __init__(self, input_size, layer_size, filter_len, c, act, Finput=None):
        self._layer_size = layer_size
        self._filter_len = filter_len
        self._input_size = input_size
        self._c = c
        self._params = None
        self._act = act
        self._Finput = Finput

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
        return (self._layer_size, self._layer_size, self._layer_size, self._layer_size, self._layer_size, self._layer_size)

    @property
    def output_size(self):
        return (self._layer_size, self._layer_size, self._layer_size, self._filter_len * self._input_size, self._layer_size)

    def _init_parameters(self):
        if self._Finput is not None: return (self._Finput, )
        
        c = self._c
        return (
            tf.get_variable("F", [self._filter_len * self._input_size, self._layer_size], 
                initializer=tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
            ),
            tf.get_variable("Fc", [self._layer_size, self._layer_size], 
                initializer=tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
            )
        )

    def __call__(self, input, state, scope=None):
        with tf.variable_scope(scope or type(self).__name__):
            ### init
            if self._params is None:
                self._params = self._init_parameters()

            x = input[0]
            c = self._c
            
            batch_size, filter_len, input_size = x.get_shape().as_list()
            
            u, a, a_m, fb_m, dF, dFc = state
            
            F = self._params[0]
            
            # Fc = tf.matmul(tf.transpose(F), F) - tf.eye(self._layer_size)
            Fc = self._params[1]

            x_flat = tf.reshape(x, (batch_size, filter_len * input_size))

            #### logic
                        
            new_fb = tf.matmul(a, Fc) 
            du = - u + tf.matmul(x_flat, F)
            
            if c.smooth_feedback:
                du = du - fb_m
            else:
                du = du - new_fb
            

            if c.adaptive:
                du  = du - a_m

            # du = tf.clip_by_value(du, -5.0, 5.0)
            
            new_u = u + c.epsilon * du / c.tau
            
            # new_u = tf.clip_by_value(new_u, -5.0, 5.0)

            if c.adaptive_threshold:
                threshold = a_m
            else:
                threshold = c.lam
       
            # new_a = tf.nn.relu()
            new_a = c.act_factor * self._act(new_u - threshold)
            new_a_m = a_m + c.epsilon *(c.adapt * new_a - a_m)/c.tau_m
            new_fb_m = fb_m + c.epsilon * (c.fb_factor * new_fb - fb_m)/c.tau_fb

            #### learning
            
            
            x_hat_flat = tf.matmul(new_a, tf.transpose(F))
            
            error_part = x_hat_flat - x_flat
            
            new_dF = dF + c.grad_accum_rate * tf.matmul(tf.transpose(error_part), new_a)
            
            new_dFc = dFc + c.grad_accum_rate * ( - tf.matmul(
                tf.transpose(new_a), new_a
            ))
            
            # for v in (new_u, new_a, new_a_m, new_dF, new_dFc):
                # print v.get_name(), v.get_shape()

            return (new_u, new_a, new_a_m, x_hat_flat, fb_m), (new_u, new_a, new_a_m, new_fb_m, new_dF, new_dFc)

    @property
    def F(self):
        assert self._params is not None
        return tf.reshape(self._params[0], (self._filter_len, self._input_size, self._layer_size))

    @property
    def F_flat(self):
        assert self._params is not None
        return self._params[0]

    @property
    def Fc(self):
        assert self._params is not None
        return self._params[1]


class LCAScalarCell(RNNCell):
    def __init__(self, input_size, layer_size, c, act, Finput=None):
        self._layer_size = layer_size
        self._input_size = input_size
        self._c = c
        self._act = act
        
        self._Finput = Finput
        self._params = None
        

    @property
    def state_size(self):
        return (self._layer_size, self._layer_size, self._layer_size, self._layer_size, self._layer_size)

    @property
    def output_size(self):
        return (self._layer_size, self._layer_size, self._layer_size, self._input_size, self._layer_size)


    @property
    def layer_size(self):
        return self._layer_size
    
    @property
    def input_size(self):
        return self._input_size


    def _init_parameters(self):
        if self._Finput is not None: return (self._Finput, )
        
        c = self._c
        return (
            tf.get_variable("F", [self._input_size, self._layer_size], 
                initializer=tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
            ),
            tf.get_variable("Fc", [self._layer_size, self._layer_size], 
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
            
            batch_size, input_size = x.get_shape().as_list()
            
            u, a, a_m, fb_m, dF, dFc = state
            F = self._params[0]
            
            Fc = self._params[1]
            # Fc = tf.matmul(tf.transpose(F), F) - tf.eye(self._layer_size)
            # Fc = tf.matrix_set_diag(Fc, tf.zeros(self._layer_size))
            #### logic
                        
            # du = - u + tf.matmul(x_flat, F) - 3.0*tf.matmul(a, Fc) - 20.0*a_m
            
            new_fb = tf.matmul(a, Fc)
            du = - u + tf.matmul(x, F) 
            
            if c.smooth_feedback:
                du = du - fb_m
            else:
                du = du - new_fb
            
            if c.adaptive:
                du  = du - a_m

            # du = tf.clip_by_value(du, -5.0, 5.0)
            
            new_u = u + c.epsilon * du / c.tau
            
            # new_u = tf.clip_by_value(new_u, -5.0, 5.0)
            if c.adaptive_threshold:
                threshold = a_m
            else:
                threshold = c.lam
            
            # new_a = tf.nn.relu()
            new_a = c.act_factor * self._act(new_u - threshold)
            new_a_m = a_m + c.epsilon * (c.adapt * new_a - a_m)/c.tau_m
            
            new_fb_m = fb_m + c.epsilon * (c.fb_factor * new_fb - fb_m)/c.tau_fb

            # new_a_m = (1.0 - 1.0/c.tau_m) * a_m + (1.0/c.tau_m) * new_a
            
            #### learning
            
            x_hat = tf.matmul(new_a, tf.transpose(F))
            
            error_part = x_hat - x
            new_dF = dF + c.grad_accum_rate * tf.matmul(tf.transpose(error_part), new_a)
            
            new_dFc = dFc + c.grad_accum_rate * ( - tf.matmul(
                tf.transpose(new_a), new_a
            ))
            
            return (new_a, new_u, new_a_m, x_hat, fb_m), (new_u, new_a, new_a_m, new_fb_m, new_dF, new_dFc)

    @property
    def F(self):
        assert self._params is not None
        return self._params[0]

    @property
    def Fc(self):
        assert self._params is not None
        return self._params[1]



def normalize_weights(net):
    ops = []
    for cell in net._cells:
        ops.append(tf.assign(cell.Fc, tf.nn.l2_normalize(cell.Fc, 0)))
    return tf.group(*ops)



