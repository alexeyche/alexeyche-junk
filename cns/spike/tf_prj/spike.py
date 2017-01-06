
import tensorflow as tf
from tensorflow.python.framework.function import Defun

@Defun(tf.float32, tf.float32)
def bprop(x, dy):
    return tf.sigmoid(x)

@Defun(tf.float32, grad_func=bprop)
def fprop(x):
    return x  # identity


def main(unused_argv):
    a = tf.Variable(tf.constant([-5., 4., -3., 2., 1.], dtype=tf.float32))

    with tf.Session() as sess:                                                             
        sess.run(tf.initialize_all_variables())
        a = tf.identity(a)
        grad = tf.gradients(fprop(a), [a])                                         
        result = sess.run(grad)                                                        

    print result


if __name__ == "__main__":
    tf.app.run()
