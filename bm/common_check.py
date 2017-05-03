import numpy as np

def grad_check(v, v_approx, name, fail, tol):
    error = np.sum(np.abs(v - v_approx))
    if not np.abs(error) < tol:
        error_text = "Gradient calculation error of {} is too big: {}\n\tgiven: {}\n\texpected: {}".format(
            name, error, v, v_approx
        )
        if fail:
            raise Exception(error_text)
        else:
            print error_text
