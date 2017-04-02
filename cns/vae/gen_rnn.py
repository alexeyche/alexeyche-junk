#!/usr/bin/env python

from tensorflow.python.util import nest
from tensorflow.python.ops import rnn_cell
from tensorflow.python.ops import rnn_cell_impl
from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import array_ops
from tensorflow.python.framework import dtypes
from tensorflow.python.framework import ops
from tensorflow.python.ops import tensor_array_ops
from tensorflow.python.ops import control_flow_ops

# pylint: disable=protected-access
_state_size_with_prefix = rnn_cell_impl._state_size_with_prefix
# pylint: enable=protected-access


from function import function

class ProjectionCell(rnn_cell.RNNCell):
    def __init__(self, base_cell, projection_size, activation=None):
        self._base_cell = base_cell
        self._projection_size = projection_size
        self._activation = activation

    @property
    def state_size(self):
        return self._base_cell.state_size

    @property
    def output_size(self):
        return self._projection_size

    def __call__(self, x, h, scope=None):
        new_x, new_h = self._base_cell(x, h)
        proj_x = function(new_x, name="projection", size=self._projection_size, scope=scope, act=self._activation)
        return proj_x, new_h


def _infer_state_dtype(explicit_dtype, state):
  """Infer the dtype of an RNN state.

  Args:
    explicit_dtype: explicitly declared dtype or None.
    state: RNN's hidden state. Must be a Tensor or a nested iterable containing
      Tensors.

  Returns:
    dtype: inferred dtype of hidden state.

  Raises:
    ValueError: if `state` has heterogeneous dtypes or is empty.
  """
  if explicit_dtype is not None:
    return explicit_dtype
  elif nest.is_sequence(state):
    inferred_dtypes = [element.dtype for element in nest.flatten(state)]
    if not inferred_dtypes:
      raise ValueError("Unable to infer dtype from empty state.")
    all_same = all([x == inferred_dtypes[0] for x in inferred_dtypes])
    if not all_same:
      raise ValueError(
          "State has tensors of different inferred_dtypes. Unable to infer a "
          "single representative dtype.")
    return inferred_dtypes[0]
  else:
    return state.dtype

def dynamic_gen_rnn(cell, input, time_steps, sequence_length=None, initial_state=None,
                dtype=None, parallel_iterations=None, swap_memory=False,
                scope=None):
  if not isinstance(cell, rnn_cell.RNNCell):
    raise TypeError("cell must be an instance of RNNCell")

  flat_input = nest.flatten(input)

  parallel_iterations = parallel_iterations or 32
  if sequence_length is not None:
    sequence_length = math_ops.to_int32(sequence_length)
    if sequence_length.get_shape().ndims not in (None, 1):
      raise ValueError(
          "sequence_length must be a vector of length batch_size, "
          "but saw shape: %s" % sequence_length.get_shape())
    sequence_length = array_ops.identity(  # Just to find it in the graph.
        sequence_length, name="sequence_length")

  # Create a new scope in which the caching device is either
  # determined by the parent scope, or is set to place the cached
  # Variable using the same placement as for the rest of the RNN.
  with vs.variable_scope(scope or "rnn") as varscope:
    if varscope.caching_device is None:
      varscope.set_caching_device(lambda op: op.device)
    input_shape = tuple(array_ops.shape(input_) for input_ in flat_input)
    batch_size = input_shape[0][0]

    for input_ in input_shape:
      if input_[0].get_shape() != batch_size.get_shape():
        raise ValueError("All inputs should have the same batch size")

    if initial_state is not None:
      state = initial_state
    else:
      if not dtype:
        raise ValueError("If no initial_state is provided, dtype must be.")
      state = cell.zero_state(batch_size, dtype)

    def _assert_has_shape(x, shape):
      x_shape = array_ops.shape(x)
      packed_shape = array_ops.stack(shape)
      return control_flow_ops.Assert(
          math_ops.reduce_all(math_ops.equal(x_shape, packed_shape)),
          ["Expected shape for Tensor %s is " % x.name,
           packed_shape, " but saw shape: ", x_shape])

    if sequence_length is not None:
      # Perform some shape validation
      with ops.control_dependencies(
          [_assert_has_shape(sequence_length, [batch_size])]):
        sequence_length = array_ops.identity(
            sequence_length, name="CheckSeqLen")

    input_packed = nest.pack_sequence_as(structure=input, flat_sequence=flat_input)

    (outputs, final_state) = _dynamic_gen_rnn_loop(
        cell,
        input_packed,
        state,
        time_steps,
        parallel_iterations=parallel_iterations,
        swap_memory=swap_memory,
        sequence_length=sequence_length,
        dtype=dtype)

    return (outputs, final_state)


def _dynamic_gen_rnn_loop(cell,
                      input,
                      initial_state,
                      time_steps,
                      parallel_iterations,
                      swap_memory,
                      sequence_length=None,
                      dtype=None):
  state = initial_state
  assert isinstance(parallel_iterations, int), "parallel_iterations must be int"

  state_size = cell.state_size

  flat_input = nest.flatten(input)
  flat_output_size = nest.flatten(cell.output_size)

  # Construct an initial output
  input_shape = array_ops.shape(flat_input[0])
  batch_size = input_shape[0]

  inputs_got_shape = tuple(input_.get_shape().with_rank_at_least(2)
                           for input_ in flat_input)

  const_batch_size = inputs_got_shape[0].as_list()[0]

  for shape in inputs_got_shape:
    got_batch_size = shape[0].value
    if const_batch_size != got_batch_size:
      raise ValueError(
          "Batch_size is not the same for all the elements in the input.")

  # Prepare dynamic conditional copying of state & output
  def _create_zero_arrays(size):
    size = _state_size_with_prefix(size, prefix=[batch_size])
    return array_ops.zeros(
        array_ops.stack(size), _infer_state_dtype(dtype, state))

  flat_zero_output = tuple(_create_zero_arrays(output)
                           for output in flat_output_size)
  zero_output = nest.pack_sequence_as(structure=cell.output_size,
                                      flat_sequence=flat_zero_output)

  if sequence_length is not None:
    min_sequence_length = math_ops.reduce_min(sequence_length)
    max_sequence_length = math_ops.reduce_max(sequence_length)

  time = array_ops.constant(0, dtype=dtypes.int32, name="time")

  with ops.name_scope("dynamic_rnn") as scope:
    base_name = scope

  def _create_ta(name, dtype):
    return tensor_array_ops.TensorArray(dtype=dtype,
                                        size=time_steps,
                                        tensor_array_name=base_name + name)

  output_ta = tuple(_create_ta("output_%d" % i,
                               _infer_state_dtype(dtype, state))
                    for i in range(len(flat_output_size)))
  # input_ta = tuple(_create_ta("input_%d" % i, flat_input[0].dtype)
  #                  for i in range(len(flat_input)))

  # input_ta = tuple(ta.unstack(input_)
  #                  for ta, input_ in zip(input_ta, flat_input))

  def _time_step(time, next_input, output_ta_t, state):
    for input_, shape in zip(next_input, inputs_got_shape):
      input_.set_shape(shape)

    
    input_t = nest.pack_sequence_as(structure=input, flat_sequence=next_input)
    # import tensorflow as tf
    # input_t = tf.Print(input_t, [input_t, time], "input_t, time")

    call_cell = lambda: cell(input_t, state)

    if sequence_length is not None:
      (output, new_state) = _rnn_step(
          time=time,
          sequence_length=sequence_length,
          min_sequence_length=min_sequence_length,
          max_sequence_length=max_sequence_length,
          zero_output=zero_output,
          state=state,
          call_cell=call_cell,
          state_size=state_size,
          skip_conditionals=True)
    else:
      (output, new_state) = call_cell()

    # Pack state if using state tuples
    output = nest.flatten(output)

    output_ta_t = tuple(
        ta.write(time, out) for ta, out in zip(output_ta_t, output))

    return (time + 1, output, output_ta_t, new_state)
  
  next_input = flat_input
  _, _, output_final_ta, final_state = control_flow_ops.while_loop(
      cond=lambda time, *_: time < time_steps,
      body=_time_step,
      loop_vars=(time, next_input, output_ta, state),
      parallel_iterations=parallel_iterations,
      swap_memory=swap_memory)

  # Unpack final output if not using output tuples.
  final_outputs = tuple(ta.stack() for ta in output_final_ta)

  # Restore some shape information
  for output, output_size in zip(final_outputs, flat_output_size):
    shape = _state_size_with_prefix(
        output_size, prefix=[time_steps, const_batch_size])
    output.set_shape(shape)

  final_outputs = nest.pack_sequence_as(
      structure=cell.output_size, flat_sequence=final_outputs)

  return (final_outputs, final_state)



# pylint: disable=unused-argument
def _rnn_step(
    time, sequence_length, min_sequence_length, max_sequence_length,
    zero_output, state, call_cell, state_size, skip_conditionals=False):
  """Calculate one step of a dynamic RNN minibatch.

  Returns an (output, state) pair conditioned on the sequence_lengths.
  When skip_conditionals=False, the pseudocode is something like:

  if t >= max_sequence_length:
    return (zero_output, state)
  if t < min_sequence_length:
    return call_cell()

  # Selectively output zeros or output, old state or new state depending
  # on if we've finished calculating each row.
  new_output, new_state = call_cell()
  final_output = np.vstack([
    zero_output if time >= sequence_lengths[r] else new_output_r
    for r, new_output_r in enumerate(new_output)
  ])
  final_state = np.vstack([
    state[r] if time >= sequence_lengths[r] else new_state_r
    for r, new_state_r in enumerate(new_state)
  ])
  return (final_output, final_state)

  Args:
    time: Python int, the current time step
    sequence_length: int32 `Tensor` vector of size [batch_size]
    min_sequence_length: int32 `Tensor` scalar, min of sequence_length
    max_sequence_length: int32 `Tensor` scalar, max of sequence_length
    zero_output: `Tensor` vector of shape [output_size]
    state: Either a single `Tensor` matrix of shape `[batch_size, state_size]`,
      or a list/tuple of such tensors.
    call_cell: lambda returning tuple of (new_output, new_state) where
      new_output is a `Tensor` matrix of shape `[batch_size, output_size]`.
      new_state is a `Tensor` matrix of shape `[batch_size, state_size]`.
    state_size: The `cell.state_size` associated with the state.
    skip_conditionals: Python bool, whether to skip using the conditional
      calculations.  This is useful for `dynamic_rnn`, where the input tensor
      matches `max_sequence_length`, and using conditionals just slows
      everything down.

  Returns:
    A tuple of (`final_output`, `final_state`) as given by the pseudocode above:
      final_output is a `Tensor` matrix of shape [batch_size, output_size]
      final_state is either a single `Tensor` matrix, or a tuple of such
        matrices (matching length and shapes of input `state`).

  Raises:
    ValueError: If the cell returns a state tuple whose length does not match
      that returned by `state_size`.
  """

  # Convert state to a list for ease of use
  flat_state = nest.flatten(state)
  flat_zero_output = nest.flatten(zero_output)

  def _copy_one_through(output, new_output):
    copy_cond = (time >= sequence_length)
    return _on_device(
        lambda: array_ops.where(copy_cond, output, new_output),
        device=new_output.op.device)

  def _copy_some_through(flat_new_output, flat_new_state):
    # Use broadcasting select to determine which values should get
    # the previous state & zero output, and which values should get
    # a calculated state & output.
    flat_new_output = [
        _copy_one_through(zero_output, new_output)
        for zero_output, new_output in zip(flat_zero_output, flat_new_output)]
    flat_new_state = [
        _copy_one_through(state, new_state)
        for state, new_state in zip(flat_state, flat_new_state)]
    return flat_new_output + flat_new_state

  def _maybe_copy_some_through():
    """Run RNN step.  Pass through either no or some past state."""
    new_output, new_state = call_cell()

    nest.assert_same_structure(state, new_state)

    flat_new_state = nest.flatten(new_state)
    flat_new_output = nest.flatten(new_output)
    return control_flow_ops.cond(
        # if t < min_seq_len: calculate and return everything
        time < min_sequence_length, lambda: flat_new_output + flat_new_state,
        # else copy some of it through
        lambda: _copy_some_through(flat_new_output, flat_new_state))

  # TODO(ebrevdo): skipping these conditionals may cause a slowdown,
  # but benefits from removing cond() and its gradient.  We should
  # profile with and without this switch here.
  if skip_conditionals:
    # Instead of using conditionals, perform the selective copy at all time
    # steps.  This is faster when max_seq_len is equal to the number of unrolls
    # (which is typical for dynamic_rnn).
    new_output, new_state = call_cell()
    nest.assert_same_structure(state, new_state)
    new_state = nest.flatten(new_state)
    new_output = nest.flatten(new_output)
    final_output_and_state = _copy_some_through(new_output, new_state)
  else:
    empty_update = lambda: flat_zero_output + flat_state
    final_output_and_state = control_flow_ops.cond(
        # if t >= max_seq_len: copy all state through, output zeros
        time >= max_sequence_length, empty_update,
        # otherwise calculation is required: copy some or all of it through
        _maybe_copy_some_through)

  if len(final_output_and_state) != len(flat_zero_output) + len(flat_state):
    raise ValueError("Internal error: state and output were not concatenated "
                     "correctly.")
  final_output = final_output_and_state[:len(flat_zero_output)]
  final_state = final_output_and_state[len(flat_zero_output):]

  for output, flat_output in zip(final_output, flat_zero_output):
    output.set_shape(flat_output.get_shape())
  for substate, flat_substate in zip(final_state, flat_state):
    substate.set_shape(flat_substate.get_shape())

  final_output = nest.pack_sequence_as(
      structure=zero_output, flat_sequence=final_output)
  final_state = nest.pack_sequence_as(
      structure=state, flat_sequence=final_state)

  return final_output, final_state


