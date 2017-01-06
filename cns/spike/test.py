
import tensorflow as tf
from tensorflow.python.framework.function import Defun

# @Defun(tf.float32, tf.float32)
# def bprop(x, dy):
#   return tf.sigmoid(x)

# @Defun(tf.float32, grad_func=bprop)
# def fprop(x):
#   return x  # identity


def my_op_grad(op, grad): ### instead of my_op_grad(x)                                                  
    return tf.sigmoid(op.inputs[0])                                              

@Defun(tf.float32, python_grad_func=my_op_grad, func_name="my_op")
def my_op(a):                                                                    
    return tf.identity(a)   


a = tf.Variable(tf.constant([-5., 4., -3., 2., 1.], dtype=tf.float32))         
sess = tf.Session()                                                            
sess.run(tf.global_variables_initializer())                                        

a = tf.identity(a) #workaround for bug github.com/tensorflow/tensorflow/issues/3710

grad = tf.gradients(my_op(a), [a])[0]                                          
result = sess.run(grad)                                                        

print(result)                                                                  

sess.close()     
