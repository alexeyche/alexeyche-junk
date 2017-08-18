import numpy as np


seed = 10

np.random.seed(seed)

x_v, y_v = make_classification(        
    n_samples=n_samples,
    n_features=n_features, 
    random_state=seed,
    n_redundant=0,
    n_repeated=0,
)


