#!/usr/bin/env python

import numpy as np
import os
os.environ["KMP_DUPLICATE_LIB_OK"] = "TRUE"
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
telcom = pd.read_csv(pj(AML_WD, "datasets/WA_Fn-UseC_-Telco-Customer-Churn.csv"))

replace_cols = [
    'OnlineSecurity', 'OnlineBackup', 'DeviceProtection',
    'TechSupport', 'StreamingTV', 'StreamingMovies'
]

for i in replace_cols:
    telcom[i] = telcom[i].replace({'No internet service': 'No'})

# eplace values
telcom["SeniorCitizen"] = telcom["SeniorCitizen"].replace({1: "Yes", 0: "No"})

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

telcom["tenure_group"] = telcom.apply(lambda telcom: tenure_lab(telcom), axis = 1)

# telcom = telcom.drop("tenure", axis=1)
# # telcom = telcom.drop("TotalCharges", axis=1)
# telcom = telcom.drop("customerID", axis=1)
# telcom.TotalCharges = pd.to_numeric(telcom.TotalCharges, errors='coerce')
# telcom['Churn'].replace(to_replace='Yes', value=1, inplace=True)
# telcom['Churn'].replace(to_replace='No', value=0, inplace=True)
# telcom = pd.get_dummies(telcom)
# telcom.dropna(inplace = True)


fp = FeaturePool.from_dataframe(telcom)

seed = 5

clean = Pipeline(
    TParse(),
    TCleanPool(),
    TSummary(),
)

te = Pipeline(
    TPreprocessPool(),
    TSummary(),
    TCleanRedundantFeatures(correlation_bound=0.99),
)

cfp = clean.run(fp)
pfp = te.run(cfp)

umap = TUmap(n_neighbors=15, min_dist=0.1, spread=10.0, metric="minkowski", random_state=1)

efp = FeaturePool(tuple(f for f in umap(pfp.features)))

TUmap.plot_embedding(efp, split_by=TLabelEncoder().transform_single(cfp["Contract"]))


t = Pipeline(
    TPreprocessPool(),
    TSummary(),
    TCleanRedundantFeatures(correlation_bound=0.99),
    TTrainTestSplit(test_size=0.3, random_state=seed),
    TSummary(),
    TSetTarget(name="Churn"),
    # TOverSampling(random_state=seed),
    # TLambda(lambda x: TSelectKBest(num_of_features=3).plot(x)),

    # TFeatureElimination(
    #     MLogReg(),
    #     num_of_features=10
    # ),
    MLogReg(random_state=seed),
    # MPool(
    #     MDecisionTree(maximum_depth=3),
    #     MLogReg(),
    #     MKNNClassifier(),
    #     MRandomForestClassifier(),
    #     MNaiveBayes(),
    #     MSVC(),
    #     MLGMBClassifier(),
    #     MXGBoostClassifier(),
    #     MCatBoost(),
    #     random_state=seed
    # ),
    VClassificationReport(),
)


# For error analysis
# cfp = clean.run(fp)
# mo = t.run(cfp)


# fm, test_x, test_y = mo.fp.test_arrays()
# test_y = test_y[:, 0]

# pred_test_y = mo.models[0].predict(test_x)
# print("Accuracy: ", np.mean(test_y == pred_test_y))
# x_res = test_x[np.where(test_y != pred_test_y)]
# y_res = np.expand_dims(test_y[np.where(test_y != pred_test_y)], 1)

# fp_res = FeaturePool.from_test_arrays(fm, x_res, y_res)


# plot_importance(
#     mo.models[0].importance,
#     mo.fp.train_predictors().names()
# )




