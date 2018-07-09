from poc.datasets import *
import tensorflow as tf

def flatten(p):
    return [pp for param in p for pp in param]

def init_parameters(input_size,net_structure,weight_factor):
    params = []
    for fan_in, fan_out in zip((input_size,) + net_structure[:-1], net_structure):
        Winit, binit = xavier_init(fan_in, fan_out, const=weight_factor)
        W = tf.Variable(Winit.T, name="W")
        b = tf.Variable(binit, name="b")

        params.append((W, b))

    return params


ds = XorDataset()

(_, input_size), (_, output_size) = ds.train_shape

weight_factor = 1.0
threshold = 0.1
net_structure = (100, output_size)

x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")

f = tf.nn.relu


p = init_parameters(input_size, net_structure, weight_factor=weight_factor)


