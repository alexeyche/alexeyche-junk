
import unittest
import pandas as pd
import numpy as np
from fe_env import *

class FiltersTest(unittest.TestCase):
    def setUp(self):
        pass

    def test_positive_zero(self):
        x = pd.DataFrame(np.zeros((30, 10)))
        f = Filters.positive()
        xf = f(x)
        self.assertEqual(xf.shape[1], 0)

    def test_positive_almost_zero(self):
        x = pd.DataFrame(np.zeros((30, 10)) + 1e-08)
        f = Filters.positive()
        xf = f(x)
        self.assertEqual(xf.shape[1], 10)

    def test_no_greater_than(self):
        x = pd.DataFrame(
            np.concatenate(
                (
                    10.0 * np.ones((30, 5)),
                    5.0 * np.ones((30, 5))
                ),
                axis=1
            )
        )

        f = Filters.no_greater_than(10.0)
        xf = f(x)
        self.assertEqual(xf.shape[1], 5)
        self.assertAlmostEqual(np.mean(xf.values), 5.0)

if __name__ == '__main__':
    unittest.main()
