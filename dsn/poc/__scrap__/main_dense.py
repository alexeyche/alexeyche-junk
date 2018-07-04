
from poc.datasets import *
from poc.__scrap__.act import *
from poc.__scrap__.common import *

np.random.seed(11)
# ds = XorDataset()

ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape

n = Network(
    input_size=input_size,
    batch_size=ds.train_batch_size,
    test_batch_size=ds.test_batch_size,
    net_structure=(250, output_size),
    weight_factor=1.0,
    net_act=Threshold(threshold=0.1),
    # net_act=Relu(),
    output_act=Sigmoid(),
    lrate=0.00002,
    sparse=False,
    lil_epsilon=1.0,
    big_epsilon=10.0
)

n.train(ds, epochs=1000, print_freq=10)