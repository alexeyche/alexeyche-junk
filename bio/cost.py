import numpy as np
from common_check import grad_check, check_matrix

class Cost(object):
    @staticmethod
    def get_approx_grad(x):
        raise NotImplementedError

    def __call__(self, y, yt):
        raise NotImplementedError

    def grad(self, y, yt):
        raise NotImplementedError    

    def approx_grad(self, y, yt, epsilon=1e-05):
        return check_matrix(y, lambda inp: self(inp, yt), epsilon=epsilon)




class MseCost(Cost):
    def __call__(self, y, yt):
        return np.square(y - yt)

    def grad(self, y, yt):
        return 2.0 * (y - yt)


def test_cost_grad(cost, y, yt, epsilon=1e-05, tol=1e-05, fail=True):
    return grad_check(
        cost.grad(y, yt), 
        cost.approx_grad(y, yt, epsilon=epsilon), 
        "cost", 
        fail, tol
    )


if __name__ == '__main__':
    y = np.random.random((10, 10))
    yt = np.random.random((10, 10))
    test_cost_grad(MseCost(), y, yt)