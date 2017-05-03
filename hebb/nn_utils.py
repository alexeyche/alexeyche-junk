
import numpy as np

def initialize_layer(n_in, n_out):
    rng = np.random.RandomState()
    W_values = np.asarray(
        rng.uniform(
            low=-np.sqrt(6. / (n_in + n_out)),
            high=np.sqrt(6. / (n_in + n_out)),
            size=(n_in, n_out)
        ),
        dtype=np.float32
    )
    return W_values


