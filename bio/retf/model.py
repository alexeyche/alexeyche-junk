
import tensorflow as tf
from tensorflow.contrib.rnn import RNNCell as RNNCell


class DnnCell(RNNCell):
    def __init__(self, size_tuple, act, is_output_cell, c):
        self._size_tuple = size_tuple
        self._act = act
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
            state_size
        ) + ((state_size, ) if not self._is_output_cell else tuple())

    @property
    def output_size(self):
        input_size, state_size, feedback_size = self._size_tuple
        
        return (state_size, state_size,)

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
        )

    def __call__(self, input, state, scope=None):
        with tf.variable_scope(scope or type(self).__name__):
            if self._params is None:
                self._params = self._init_parameters(*self._size_tuple)

            c = self._c

            Wff, bias_ff, Wfb = self._params
            
            input_feedforward, input_feedback, modulation = input
            
            basal_state, basal_state_m, soma_state, soma_state_m, rate_m = state[:5]
            
            new_basal_state = basal_state + c.dt * (- basal_state/c.tau_syn + input_feedforward)
            

            ff = ((bias_ff + tf.matmul(new_basal_state, Wff)) - soma_state)/c.tau_b
            
            if self._is_output_cell:
                gE = input_feedback
                gI = 1.0 - input_feedback    
                
                fb = gE * (c.Ee - soma_state) + gI * (c.Ei - soma_state)
            else:
                apical = tf.matmul(input_feedback, Wfb)
                
                apical_m = state[5]
                
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

            return (
                rate, 
                spikes,
            ), (
                new_basal_state, 
                new_basal_state_m, 
                new_soma_state, 
                new_soma_state_m, 
                new_rate_m
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

                xit = it[-1]

                if last_cell:
                    new_output_f = output_f + c.dt * (- output_f/c.tau_syn + xit)

            return tuple(new_outputs), tuple(new_states) + (new_output_f,)

    @property
    def cells(self):
        return self._cells
    
