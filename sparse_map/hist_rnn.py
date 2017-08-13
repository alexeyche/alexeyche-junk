import tensorflow as tf
import collections

def cat_hist(hist, new, dim):
    assert len(new.get_shape().as_list()) == len(hist.get_shape().as_list())-1
    new = tf.expand_dims(new, dim)
    source_shape = hist.get_shape().as_list()
    
    start_slice = [0]*len(source_shape)
    start_slice[dim] = 1
    end_slice = source_shape[:]
    end_slice[dim] -= 1

    hist_sliced = tf.slice(hist, start_slice, end_slice)
    return tf.concat([hist_sliced, new], dim)



def rnn_with_hist_loop_fn(inputs, sequence_length, initial_state, window):
    batch_size = sequence_length.get_shape()[0]
    input_shape = inputs.get_shape().as_list()
    assert len(input_shape) == 3, "Expecting input with shape [T, batch_size, dim_size]"
    assert input_shape[1] == batch_size, "Expecting sequence length with shape [batch_size] according to input"
    max_time = input_shape[0]
    input_size = input_shape[2]

    inputs_ta = tf.TensorArray(dtype=tf.float32, size=max_time)
    inputs_ta = inputs_ta.unstack(inputs)

    def loop_fn(time, cell_output, cell_state, loop_state):
        emit_output = cell_output  # == None for time == 0
        
        if cell_output is None:  # time == 0
            next_cell_state = initial_state
        else:
            next_cell_state = cell_state
        
        elements_finished = (time >= sequence_length)
        
        finished = tf.reduce_all(elements_finished)
        
        next_input = tf.cond(
            finished,
            lambda: tf.zeros([batch_size, input_size], dtype=tf.float32),
            lambda: inputs_ta.read(time))

        
        if loop_state is None:
            next_input = tf.expand_dims(next_input, 1)
            next_input = tf.pad(next_input, [[0,0], [window-1, 0], [0,0]])
        else:
            next_input = cat_hist(loop_state, next_input, 1)
        
        next_loop_state = next_input
        
        return (elements_finished, (next_input,), next_cell_state, emit_output, next_loop_state)
    return loop_fn
