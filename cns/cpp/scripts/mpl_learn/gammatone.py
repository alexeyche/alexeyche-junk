
import logging
import numpy as np

def safe_log(v):
    if v == 0:
        return 0
    return np.log(v)

def gammatone(**kwargs):
    a = kwargs["a"]
    b = kwargs["b"]
    n = kwargs["n"]
    fnum = kwargs["fnum"]
    lb = kwargs["lb"]
    hb = kwargs["hb"]
    L = kwargs["L"]
    Tmax = kwargs["Tmax"]
    phi = kwargs["phi"]
    log_freq = kwargs["log_freq"]

    time = np.linspace(0, Tmax, L)
    logging.info("{}".format(str(kwargs)))
    if log_freq:
        freq = np.exp(np.linspace(safe_log(lb), safe_log(hb), fnum))
    else:
        freq = np.linspace(lb, hb, fnum)

    filt = np.zeros( (fnum, L) )
    for fi, f in enumerate(freq):
        filt[fi,] = a * np.power(time, n) * np.exp(-2*np.pi*b*time) * np.cos(2*np.pi*f*time + phi)
    return filt        


