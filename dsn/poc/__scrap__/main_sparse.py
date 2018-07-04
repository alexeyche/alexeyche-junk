
from poc.datasets import *
from poc.__scrap__.act import *
from poc.__scrap__.common import *

np.random.seed(11)
ds = XorDataset()

(_, input_size), (_, output_size) = ds.train_shape



n = Network(
    input_size=input_size,
    batch_size=ds.train_batch_size,
    test_batch_size=ds.test_batch_size,
    net_structure=(200, output_size),
    weight_factor=1.0,
    net_act=ThresholdSparse(threshold=0.15),
    # net_act=Relu(),
    output_act=SigmoidSparse(),
    lrate=0.00005,
    sparse=True,
    lil_epsilon=1.0,
    big_epsilon=10.0
)

n.train(ds, epochs=1, print_freq=10)


x, y = ds.next_train_batch()
v = np.dot(csr_matrix(x), n.p[0][0])