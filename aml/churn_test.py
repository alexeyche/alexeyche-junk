#!/usr/bin/env python

import numpy as np
import os
from os.path import join as pj
import pandas as pd
from matplotlib import pyplot as plt
import logging
import sys

from feature import Feature, FeaturePool


def add_coloring_to_emit_ansi(fn):
    # add methods we need to the class
    def new(*args):
        levelno = args[1].levelno
        if (levelno >= 50):
            color = '\x1b[31m'  # red
        elif (levelno >= 40):
            color = '\x1b[31m'  # red
        elif (levelno >= 30):
            color = '\x1b[33m'  # yellow
        elif (levelno >= 20):
            color = '\x1b[32m'  # green
        elif (levelno >= 10):
            color = '\x1b[35m'  # pink
        else:
            color = '\x1b[0m'  # normal
        args[1].msg = color + args[1].msg + '\x1b[0m'  # normal
        return fn(*args)

    return new


logging.StreamHandler.emit = add_coloring_to_emit_ansi(logging.StreamHandler.emit)




AML_WD = pj(os.environ["HOME"], "aml")


# https://www.kaggle.com/pavanraj159/telecom-customer-churn-prediction
data = pd.read_csv(pj(AML_WD, "datasets/WA_Fn-UseC_-Telco-Customer-Churn.csv"))

fp = FeaturePool.from_dataframe(data)



