
import numpy as np

num_of_points = 1000
means = 100 + 0.4 * np.random.binomial(1, 0.3, num_of_points)

data = means + np.random.randn(num_of_points)
