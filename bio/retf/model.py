
import tensorflow as tf
from tensorflow.contrib.rnn import RNNCell as RNNCell

def batch_outer(left, right):
    return tf.matmul(tf.expand_dims(left, 2), tf.expand_dims(right, 1))


def sigmoid_deriv(x):
    s = tf.sigmoid(x)
    return s * (1.0 - s)

class DnnCell(RNNCell):
    def __init__(self, size_tuple, is_output_cell, c):
        self._size_tuple = size_tuple
        
        self._act = tf.sigmoid
        self._act_deriv = sigmoid_deriv

        self._is_output_cell = is_output_cell
        self._c = c

        self._params = None

    @property
    def state_size(self):
        input_size, state_size, feedback_size = self._size_tuple
        return (
            input_size, 
            input_size,
            state_size, 
            state_size,
            state_size,
            (input_size, state_size),
            state_size,
            # (feedback_size, state_size),
        ) + ((state_size, ) if not self._is_output_cell else tuple())

    @property
    def output_size(self):
        input_size, state_size, feedback_size = self._size_tuple
        
        return (
            state_size, 
            state_size, 
            input_size, 
            state_size, 
            feedback_size, 
            state_size
        )
        # return (state_size, state_size)

    @property
    def params(self):
        return self._params
    

    def _init_parameters(self, input_size, state_size, feedback_size):        
        c = self._c
        
        return (
            tf.get_variable("Wff", [input_size, state_size], 
                initializer=tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
            ),
            tf.get_variable("bias_ff", [state_size], 
                initializer=tf.zeros_initializer()
            ),
            tf.get_variable("Wfb", [feedback_size, state_size], 
                initializer=tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
            ),
            tf.get_variable("Wff_sign", [input_size, state_size], 
                initializer=tf.random_uniform_initializer()
            ),
            tf.get_variable("Wfb_sign", [feedback_size, state_size], 
                initializer=tf.random_uniform_initializer()
            ),
        )

    def __call__(self, input, state, scope=None):
        with tf.variable_scope(scope or type(self).__name__):
            if self._params is None:
                self._params = self._init_parameters(*self._size_tuple)

            c = self._c

            Wff, bias_ff, Wfb, Wff_sign, Wfb_sign = self._params
            
            Wff_sign = tf.cast(tf.less(Wff_sign, 0.5), Wff_sign.dtype)*2.0 - 1.0
            Wfb_sign = tf.cast(tf.less(Wfb_sign, 0.5), Wfb_sign.dtype)*2.0 - 1.0

            input_feedforward, input_feedback, modulation = input
            
            basal_state, basal_state_m, soma_state, soma_state_m, rate_m, dW, dbias = state[:7] #, dWfb = state[:8]
            
            new_basal_state = basal_state + c.dt * (- basal_state/c.tau_syn + input_feedforward)
            
            ff = ((bias_ff + tf.matmul(new_basal_state, Wff_sign * tf.nn.softplus(Wff))) - soma_state)/c.tau_b
            
            if self._is_output_cell:
                gE = input_feedback
                gI = 1.0 - input_feedback    
                
                fb = gE * (c.Ee - soma_state) + gI * (c.Ei - soma_state)
            else:
                apical = tf.matmul(input_feedback, Wfb_sign * tf.nn.softplus(Wfb))
                
                apical_m = state[-1]
                
                new_apical_m = (1.0 - c.alpha) * apical_m + c.alpha * apical
                
                fb = (apical - soma_state)/c.tau_a


            new_soma_state = soma_state + c.dt * (- soma_state/c.tau_c + ff + modulation * fb)
            
            rate = c.lambda_max * self._act(new_soma_state)

            spikes = tf.cast(
                tf.less(
                    tf.random_uniform((c.batch_size, self._size_tuple[1])), 
                    c.dt * rate
                ), 
                rate.dtype
            )
            
            new_basal_state_m = (1.0 - c.alpha) * basal_state_m + c.alpha * new_basal_state
            new_soma_state_m = (1.0 - c.alpha) * soma_state_m + c.alpha * new_soma_state
            new_rate_m = (1.0 - c.alpha) * rate_m + c.alpha * rate

            bipolar_mod = 2.0 * (modulation - 0.5)
            

            target_rate = new_rate_m if self._is_output_cell else new_apical_m

            # grad = tf.gradients([rate], [new_soma_state])[0]

            deriv_part = - bipolar_mod * target_rate * self._act_deriv(new_rate_m)
            
            dbias += deriv_part
            dW += batch_outer(new_basal_state, deriv_part)
            
            # if not self._is_output_cell:
            #     dWfb += batch_outer(input_feedback, - bipolar_mod * new_apical_m)
            
            return (
                rate, 
                spikes,
                new_basal_state,
                apical if not self._is_output_cell else soma_state,
                input_feedback,
                new_soma_state,
            ), (
                new_basal_state, 
                new_basal_state_m, 
                new_soma_state, 
                new_soma_state_m, 
                new_rate_m,
                dW,
                dbias,
                # dWfb,
            ) + ((new_apical_m, ) if not self._is_output_cell else tuple())


class DnnNet(RNNCell):
    def __init__(self, *cells):
        self._cells = cells
        self._c = self._cells[0]._c

    @property
    def output_size(self):
        return tuple(cell.output_size for cell in self._cells)

    @property
    def state_size(self):
        return tuple(cell.state_size for cell in self._cells) + (self._cells[-1].state_size,)

    def __call__(self, input, states, scope=None):
        with tf.variable_scope(scope or type(self).__name__):
            c = self._c
        
            xt, yt, mod = input

            output_f = states[-1]

            xit = xt
            
            new_states = []
            new_outputs = []
            for i, (cell, state) in enumerate(zip(self._cells, states[:-1])):
                last_cell = i == len(self._cells)-1
                
                it, ns = cell(
                    (
                        xit, 
                        output_f if not last_cell else yt, 
                        mod
                    ), 
                    state,
                    scope="cell_{}".format(i)
                )

                new_outputs.append(it)
                new_states.append(ns)

                xit = it[1] # spikes

                if last_cell:
                    new_output_f = output_f + c.dt * (- output_f/c.tau_syn + xit)

            return tuple(new_outputs), tuple(new_states) + (new_output_f,)

    @property
    def cells(self):
        return self._cells
    
