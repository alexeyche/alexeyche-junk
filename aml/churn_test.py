#!/usr/bin/env python

import numpy as np
import os
from os.path import join as pj
import pandas as pd
from matplotlib import pyplot as plt
import logging
import sys

from util import *

from feature import Feature
from feature_pool import FeaturePool
from pipeline import Pipeline

from transform import *
from model import *
from metric import *
from validation import *
from plot_utils import *

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


logging.basicConfig(
    stream=sys.stderr,
    level=logging.DEBUG, format="%(asctime)s | %(levelname)s | %(message)s"
)
logging.StreamHandler.emit = add_coloring_to_emit_ansi(
    logging.StreamHandler.emit
)


AML_WD = pj(os.environ["HOME"], "aml")


# https://www.kaggle.com/pavanraj159/telecom-customer-churn-prediction
data = pd.read_csv(pj(AML_WD, "datasets/WA_Fn-UseC_-Telco-Customer-Churn.csv"))


fp = FeaturePool.from_dataframe(data)

seed = 0

t = Pipeline(
    TParseAndClean(),
    TSummary(),
    TCleanPool(),
    TPreprocessPool(),
    TSummary(),
    TCleanRedundantFeatures(),
    TTrainTestSplit(test_size=0.3, random_state=seed),
    TSummary(),
    TSetTarget(name="Churn"),
    TOverSampling(random_state=seed),
    # TLambda(lambda x: TSelectKBest(num_of_features=3).plot(x)),

    # TFeatureElimination(
    #     MLogReg(),
    #     num_of_features=10
    # ),
    MPool(
        MDecisionTree(maximum_depth=3),
        MLogReg(random_state=seed),
        MKNNClassifier(),
        MRandomForestClassifier(random_state=seed),
        MNaiveBayes(),
        MSVC(),
        MLGMBClassifier(random_state=seed),
        MXGBoostClassifier(random_state=seed),
    ),
    VClassificationReport()
)

o = t.run(fp)

o.fp.importance(o.models[-1])







