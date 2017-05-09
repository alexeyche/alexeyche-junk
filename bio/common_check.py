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
    return v, v_approx

def check_matrix(x, f, epsilon):
    df = np.zeros(x.shape)
    for i in xrange(df.shape[0]):
        for j in xrange(df.shape[1]):
            dx = np.zeros(df.shape)
            dx[i, j] = epsilon
    
            fl = f(x - dx)
            fr = f(x + dx)

            df[i, j] = np.sum((fr - fl)/(2.0*epsilon))
    return df