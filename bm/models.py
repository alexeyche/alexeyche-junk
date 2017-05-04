
import numpy as np
from common_check import grad_check

def batch_inner(left, right):
    return np.asarray([np.inner(left[i], right[i]) for i in xrange(left.shape[0])])


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




class Model(object):
    def __init__(self, act, W, b):
        self.W = W
        self.b = b
        self.act = act

    def get_p(self, W=None, b=None):
        if W is None:
            W = self.W
        if b is None:
            b = self.b
        return W, b

    def __call__(self, x, W=None, b=None, V=None):
        raise NotImplementedError

    def grad(self, V):
        raise NotImplementedError
        
    def dynamics(self, x):
        raise NotImplementedError

    @property
    def net_size(self):
        return self.W.shape[0]

    def approx_grad(self, x, epsilon=1e-05):
        W, b = self.get_p()        
        V = self.act(x)
        
        dW_approx = np.zeros(W.shape)
        db_approx = np.zeros(b.shape)
        x_one_batch = np.asarray([x[0,:]])
        dx_approx = np.zeros(x_one_batch.shape[1])

        for i in xrange(W.shape[0]):
            dbe = np.zeros(b.shape)
            dbe[i] = epsilon

            lEb = self(x, V=V, b=b - dbe)
            rEb = self(x, V=V, b=b + dbe)

            db_approx[i] = np.sum((rEb - lEb)/(2.0*epsilon))

            dxe = np.zeros(x.shape[1])
            dxe[i] = epsilon

            lEx = self(x_one_batch - dxe)
            rEx = self(x_one_batch + dxe)

            dx_approx[i] = np.sum((rEx - lEx)/(2.0*epsilon))

            for j in xrange(W.shape[1]):
                dWe = np.zeros(W.shape)
        
                dWe[i, j] = epsilon
                
                lEw = self(x, V=V, W=W - dWe)
                rEw = self(x, V=V, W=W + dWe)

                dW_approx[i, j] = np.sum((rEw - lEw)/(2.0*epsilon))
        
        return dx_approx, dW_approx, db_approx


    def run(self, x, n_step, x_t, y_t, cost, clamped=False, beta=1.0, epsilon=0.1):
        input_size, output_size = x_t.shape[-1], y_t.shape[-1]

        x = x.copy()
        
        for it in xrange(n_step):
            x[:, :input_size] = x_t
            
            y_hat = x[:, -output_size:]

            dx = self.dynamics(x)
            if clamped:
                dx[:, -output_size:] += beta * cost.grad(y_hat, y_t)
                
            # x = np.clip(x - epsilon * dx, 0.0, 1.0)
            x = np.clip(x - epsilon * dx, 0.0, 1.0)
            
        cost_val = np.sum(cost(y_hat, y_t), 1)
        return x, cost_val, self.act(x)




class Hopfield(Model):
    def __call__(self, x, W=None, b=None, V=None):
        V = self.act(x) if V is None else V
        W, b = self.get_p(W, b)

        return -0.5 * batch_inner(np.dot(V, W), V) - np.dot(V, b)

    def dynamics(self, x, V=None):
        V = self.act(x) if V is None else V
        W, b = self.get_p()

        return - self.act.grad(x) * (np.dot(V, W) + b)

    def grad(self, x, V=None):
        V = self.act(x) if V is None else V
        
        return -0.5 * np.dot(V.T, V), -np.sum(V, axis=0)


def sparsity_term(V, p=0.1):
    return p * np.log(np.mean(V)+1e-07) + (1.0-p) * np.log(1.0-np.mean(V) + 1e-07)


class ExpDecayHopfield(Model):
    def __call__(self, x, W=None, b=None, V=None):
        V = self.act(x) if V is None else V
        W, b = self.get_p(W, b)

        return 0.5 * batch_inner(x, x) - 0.5 * batch_inner(np.dot(V, W), V) - np.dot(V, b)

    def grad(self, x, V=None):
        V = self.act(x) if V is None else V
        
        return - 0.5 * np.dot(V.T, V), -np.sum(V, axis=0)

    def dynamics(self, x, V=None):
        V = self.act(x) if V is None else V
        W, b = self.get_p()
        
        x_s = np.mean(x[:, 2:], 1, keepdims=True) 
        I = np.concatenate([np.zeros((x.shape[0], 2)), x_s - x[:, 2:]], 1)
        
        return x + self.act.grad(x) * ( - np.dot(V, W) - b ) #+ (I - 0.1)
        

def test_model_grad(model, x, epsilon=1e-05, tol=1e-05, fail=True):
    dW, db = model.grad(x)
    assert dW.shape[0] == db.shape[0]
    dx = model.dynamics(np.asarray([x[0, :]]))

    dx_approx, dW_approx, db_approx = model.approx_grad(x, epsilon=epsilon)
    
    grad_check(dW, dW_approx, "weights", fail, tol)
    grad_check(db, db_approx, "bias", fail, tol)
    grad_check(dx, dx_approx, "dynamics", fail, tol)

    return dW, dW_approx, db, db_approx, dx, dx_approx



if __name__ == '__main__':
    from activation import ClipActivation
    from util import shl, shm, shs

    act = ClipActivation()

    W = initialize_layer(10, 10)
    W = (W + W.T)/2.0

    b = np.random.random(10)
    model = ExpDecayHopfield(act, W, b)

    x = -0.5 + 2.0 * np.random.random((2, 10))

    test_model_grad(model, x)
