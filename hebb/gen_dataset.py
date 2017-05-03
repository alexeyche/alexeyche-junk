

from sklearn.datasets import make_classification
import sys
sys.path.insert(0, "/home/alexeyche/prog/alexeyche-junk/cns/tf_dnn")
from util import xavier_init, xavier_vec_init, shl, shm, shs


x_values, y_values = make_classification(
    n_samples=1000,
    n_features=2, 
    n_informative=2, 
    n_redundant=0, 
    n_repeated=0,
    n_clusters_per_class=2,
    n_classes=2,
    scale=0.1,
    shift=5.0
)


shs(x_values, labels=y_values)