
import tensorflow as tf


def threshold_k(u, p):
    def slices_to_dims(slice_indices):
      """
      Args:
        slice_indices: An [N, k] Tensor mapping to column indices.
      Returns:
        An index Tensor with shape [N * k, 2], corresponding to indices suitable for
        passing to SparseTensor.
      """
      slice_indices = tf.cast(slice_indices, tf.int64)
      num_rows = tf.shape(slice_indices, out_type=tf.int64)[0]
      row_range = tf.range(num_rows)
      item_numbers = slice_indices * num_rows + tf.expand_dims(row_range, axis=1)
      item_numbers_flat = tf.reshape(item_numbers, [-1])
      return tf.stack([item_numbers_flat % num_rows,
                       item_numbers_flat // num_rows], axis=1)


    def to_matrix(sparse_indices, values, dense_shape):
        sparse_tensor = tf.sparse_reorder(tf.SparseTensor(
            indices=sparse_indices,
            values=tf.ones(sparse_indices.get_shape().as_list()[0]),
            #values=tf.reshape(values, [-1]),
            dense_shape=dense_shape))
        return tf.sparse_tensor_to_dense(sparse_tensor)

    k = int(u.get_shape().as_list()[1] * p)
    a_val, a_idx = tf.nn.top_k(u, k)

    return to_matrix(slices_to_dims(a_idx), a_val, u.get_shape().as_list())


def sigmoid_deriv(x):
    v = tf.sigmoid(x)
    return v * (1.0 - v)

def ltd(a_ff, a_fb):
    ltd = tf.zeros(tf.shape(a_ff))
    ltd = tf.where(a_fb < 1e-10, a_ff, ltd)
    return ltd

def relu_deriv(x):
    a = tf.nn.relu(x)
    da = tf.where(a > 0.0, tf.ones(tf.shape(a)), tf.zeros(tf.shape(a)))
    return da

def sigmoid_inv(x):
    return tf.log(x/(1.0 - x + 1e-09) + 1e-09)

def threshold(x):
    a = tf.nn.relu(x)
    return tf.where(a > 0.0, tf.ones(tf.shape(a)), tf.zeros(tf.shape(a)))
