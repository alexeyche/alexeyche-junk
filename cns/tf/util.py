import logging
import sys
import numpy as np

def add_coloring_to_emit_ansi(fn):
    # add methods we need to the class
    def new(*args):
        levelno = args[0].levelno
        if(levelno>=50):
            color = '\x1b[31m' # red
        elif(levelno>=40):
            color = '\x1b[31m' # red
        elif(levelno>=30):
            color = '\x1b[33m' # yellow
        elif(levelno>=20):
            color = '\x1b[32m' # green
        elif(levelno>=10):
            color = '\x1b[35m' # pink
        else:
            color = '\x1b[0m' # normal
        args[0].msg = color + args[0].msg +  '\x1b[0m'  # normal
        return fn(*args)
    return new 



def setup_logging(logger, console_out=sys.stdout, level=logging.DEBUG, colors=True):
    logFormatter = logging.Formatter("%(asctime)s [%(levelname)s]  %(message)-100s")
    logger.setLevel(level)
    if console_out:
        consoleHandler = logging.StreamHandler(console_out)
        if colors:
        	consoleHandler.emit = add_coloring_to_emit_ansi(consoleHandler.emit)
        consoleHandler.setFormatter(logFormatter)
        logger.addHandler(consoleHandler)


def norm(data):
    data_denom = np.sqrt(np.sum(data ** 2))
    data = data/data_denom
    return data, data_denom
