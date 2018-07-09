from poc.datasets import *
from poc.__scrap_tf__.common import *

ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape

lil_epsilon = 1.0
big_epsilon = 100.0
weight_factor = 1.0
threshold = 0.1
net_structure = (2000, 2000, output_size)

tf.set_random_seed(2)


x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")


net_act = lambda u: tf.cast(tf.greater(u, threshold), tf.float32)
net_act_deriv = lambda u: 1.0/tf.square(1.0 + tf.abs(u - threshold))


# net_act = tf.nn.relu
# net_act_deriv = relu_deriv

output_act_deriv = sigmoid_deriv

loss_deriv = lambda x, y: x - y


n = Network(
	input_size=input_size, 
	net_structure=net_structure, 
	weight_factor=weight_factor, 
	net_act=net_act, 
	net_act_deriv=net_act_deriv,
	output_act=tf.nn.sigmoid,
	output_act_deriv=output_act_deriv,
	loss_deriv=loss_deriv,
	sparse=True,
    lil_epsilon=lil_epsilon,
    big_epsilon=big_epsilon,
)


opt = tf.train.AdamOptimizer(learning_rate=0.001)

a, u, da, dp = n.build_model(x, y)


apply_grad_step = opt.apply_gradients(zip(flatten(dp), flatten(n.po)))

square_loss = tf.reduce_sum(tf.square(a[-1] - y))
class_error_rate = tf.reduce_mean(tf.cast(tf.not_equal(tf.argmax(a[-1], 1), tf.argmax(y, 1)), tf.float32))
sparsity = tf.reduce_mean([tf.reduce_mean(tf.cast(tf.equal(aa, 0.0), tf.float32)) for aa in a[:-1]])

sess = tf.Session()
sess.run(tf.global_variables_initializer())


epochs = 150
print_freq = 5

METRIC_SIZE = 3
train_metrics, test_metrics = (
    np.zeros((epochs, METRIC_SIZE)),
    np.zeros((epochs, METRIC_SIZE))
)

for epoch in range(epochs):
    for _ in range(ds.train_batches_num):
        xv, yv = ds.next_train_batch()
        
        av, uv, dav, dpv, square_loss_v, class_error_rate_v, sparsity_v, _ = sess.run(
        	(a, u, da, dp, square_loss, class_error_rate, sparsity, apply_grad_step),
        	{x: xv, y: yv}
        )

        train_metrics[epoch] += (
        	square_loss_v / ds.train_batches_num, 
        	class_error_rate_v / ds.train_batches_num,
        	sparsity_v / ds.train_batches_num
        )
        

    for _ in range(ds.test_batches_num):
        xtv, ytv = ds.next_test_batch()
        
        atv, utv, datv, dptv, square_loss_t_v, class_error_rate_t_v, sparsity_t_v = sess.run(
        	(a, u, da, dp, square_loss, class_error_rate, sparsity), 
        	{x: xtv, y: ytv}
        )

        test_metrics[epoch] += (
        	square_loss_t_v / ds.test_batches_num, 
        	class_error_rate_t_v / ds.test_batches_num,
        	sparsity_t_v / ds.test_batches_num
        )

    if epoch % print_freq == 0:
        print("Epoch {}, train {:.4f} {:.4f} {:.4f}, test {:.4f} {:.4f} {:.4f}".format(
            epoch,
            train_metrics[epoch][0],
            train_metrics[epoch][1],
            train_metrics[epoch][2],
            test_metrics[epoch][0],
            test_metrics[epoch][1],
            test_metrics[epoch][2],
        ))





