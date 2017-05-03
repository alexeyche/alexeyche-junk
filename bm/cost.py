import numpy as np
from common_check import grad_check

class Cost(object):
    @staticmethod
    def get_approx_grad(x):
        raise NotImplementedError

    def __call__(self, x):
        raise NotImplementedError

    def grad(self, x):
        raise NotImplementedError    


class MseCost(Cost):
    def __call__(self, y, yt):
        return np.square(y - yt)

    def grad(self, y, yt):
        return 2.0 * (y - yt)

    def approx_grad(self, y, yt, epsilon=1e-05):
        dc = np.zeros(y.shape)
        for i in xrange(dc.shape[0]):
            for j in xrange(dc.shape[1]):
                dye = np.zeros(dc.shape)
                dye[i, j] = epsilon
        
                lc = self(y - dye, yt)
                rc = self(y + dye, yt)

                dc[i, j] = np.sum((rc - lc)/(2.0*epsilon))
        return dc


def test_cost_grad(cost, y, yt, epsilon=1e-05, tol=1e-05, fail=True):
    y = np.asarray([y[0, :]])

    dc = cost.grad(y, yt)
    
    dc_approx = cost.approx_grad(y, yt, epsilon=epsilon)
    
    grad_check(dc, dc_approx, "cost", fail, tol)

    return dc, dc_approx

if __name__ == '__main__':
    y = np.random.random((10, 10))
    yt = np.random.random((10, 10))
    test_cost_grad(MseCost(), y, yt)