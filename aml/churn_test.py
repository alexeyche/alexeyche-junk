#!/usr/bin/env python

import numpy as np
import os
from os.path import join as pj
import pandas as pd
import matplotlib as mpl
mpl.use('Qt5Agg')
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
    level=logging.INFO, format="%(asctime)s | %(levelname)s | %(message)s"
)
logging.StreamHandler.emit = add_coloring_to_emit_ansi(
    logging.StreamHandler.emit
)


AML_WD = pj(os.environ["HOME"], "aml")


# https://www.kaggle.com/pavanraj159/telecom-customer-churn-prediction
data = pd.read_csv(pj(AML_WD, "datasets/WA_Fn-UseC_-Telco-Customer-Churn.csv"))

# replace 'No internet service' to No for the following columns
replace_cols = [
    'OnlineSecurity', 'OnlineBackup', 'DeviceProtection',
    'TechSupport', 'StreamingTV', 'StreamingMovies'
]

for i in replace_cols:
    data[i] = data[i].replace({'No internet service': 'No'})

data["SeniorCitizen"] = data["SeniorCitizen"].replace({1: "Yes", 0: "No"})

def tenure_lab(data):
    if data["tenure"] <= 12:
        return "Tenure_0-12"
    elif (data["tenure"] > 12) & (data["tenure"] <= 24):
        return "Tenure_12-24"
    elif (data["tenure"] > 24) & (data["tenure"] <= 48):
        return "Tenure_24-48"
    elif (data["tenure"] > 48) & (data["tenure"] <= 60):
        return "Tenure_48-60"
    elif data["tenure"] > 60:
        return "Tenure_gt_60"
data["tenure_group"] = data.apply(lambda data: tenure_lab(data), axis = 1)



fp = FeaturePool.from_dataframe(data)

seed = 1

t = Pipeline(
    TParseAndClean(),
    TSummary(),
    TCleanPool(),
    TPreprocessPool(),
    TSummary(),
    # TCleanRedundantFeatures(),
    TTrainTestSplit(test_size=0.3, random_state=seed),
    TSummary(),
    TSetTarget(name="Churn"),
    TOverSampling(random_state=seed),
    # TLambda(lambda x: TSelectKBest(num_of_features=3).plot(x)),

    # TFeatureElimination(
    #     MLogReg(),
    #     num_of_features=10
    # ),
    # MPool(
    #     MDecisionTree(maximum_depth=3),
    #     MLogReg(random_state=seed),
    #     MKNNClassifier(),
    #     MRandomForestClassifier(random_state=seed),
    #     MNaiveBayes(),
    #     MSVC(),
    #     MLGMBClassifier(random_state=seed),
    #     MXGBoostClassifier(random_state=seed),
    # ),
    MXGBoostClassifier(random_state=seed),
    VClassificationReport(),
)

rfp = t.run(fp)

# o.fp.importance(o.models[-1])







