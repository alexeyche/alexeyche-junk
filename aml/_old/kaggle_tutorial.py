
import numpy as np
from os.path import join as pj
import pandas as pd
import matplotlib as mpl
mpl.use('Qt5Agg')
from matplotlib import pyplot as plt
import logging
import sys

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score
from sklearn.metrics import roc_auc_score
from xgboost import XGBClassifier
from imblearn.over_sampling import SMOTE

from util import *

telcom = pd.read_csv(r"/Users/aleksei/aml/datasets/WA_Fn-UseC_-Telco-Customer-Churn.csv")


# Data Manipulation

# Replacing spaces with null values in total charges column
telcom['TotalCharges'] = telcom["TotalCharges"].replace(" ", np.nan)

# Dropping null values from total charges column which contain .15% missing data
telcom = telcom[telcom["TotalCharges"].notnull()]
telcom = telcom.reset_index()[telcom.columns]

# onvert to float type
telcom["TotalCharges"] = telcom["TotalCharges"].astype(float)

# eplace 'No internet service' to No for the following columns
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

# eparating churn and non churn customers
churn = telcom[telcom["Churn"] == "Yes"]
not_churn = telcom[telcom["Churn"] == "No"]

# eparating catagorical and numerical columns
Id_col = ['customerID']
target_col = ["Churn"]


cat_cols = telcom.nunique()[telcom.nunique() < 6].keys().tolist()
cat_cols = [x for x in cat_cols if x not in target_col]
num_cols = [x for x in telcom.columns if x not in cat_cols + target_col + Id_col]
bin_cols = telcom.nunique()[telcom.nunique() == 2].keys().tolist()
multi_cols = [i for i in cat_cols if i not in bin_cols]

le = LabelEncoder()
for i in bin_cols:
    telcom[i] = le.fit_transform(telcom[i])

telcom = pd.get_dummies(data=telcom, columns=multi_cols)

std = StandardScaler()
scaled = std.fit_transform(telcom[num_cols])
scaled = pd.DataFrame(scaled, columns=num_cols)

df_telcom_og = telcom.copy()
telcom = telcom.drop(columns=num_cols, axis=1)
telcom = telcom.merge(scaled, left_index=True, right_index=True, how="left")

cols = [i for i in telcom.columns if i not in Id_col + target_col]

random_state = None



smote_X = telcom[cols]
smote_Y = telcom[target_col]

smote_train_X, smote_test_X, smote_train_Y, smote_test_Y = \
    train_test_split(smote_X, smote_Y, test_size = .3, random_state = random_state)

os = SMOTE(random_state=random_state)
os_smote_X, os_smote_Y = os.fit_sample(smote_train_X, smote_train_Y)
os_smote_X = pd.DataFrame(data=os_smote_X, columns=cols)
os_smote_Y = pd.DataFrame(data=os_smote_Y, columns=target_col)



xgc = XGBClassifier(base_score=0.5, booster='gblinear', colsample_bylevel=1,
                    colsample_bytree=1, gamma=0, learning_rate=0.01, max_delta_step=0,
                    max_depth = 3, min_child_weight=1, missing=None, n_estimators=100,
                    n_jobs=1, nthread=None, objective='binary:logistic', random_state=0,
                    reg_alpha=0, reg_lambda=1, scale_pos_weight=1, seed=None,
                    silent=True, subsample=1)


alg = xgc
testing_x, testing_y = smote_test_X, smote_test_Y
training_x, training_y = os_smote_X, os_smote_Y


alg.fit(training_x, training_y)
predictions = alg.predict(testing_x)
probabilities = alg.predict_proba(testing_x)


print("Accuracy   Score : ", accuracy_score(testing_y, predictions))
print("Area under curve : ", roc_auc_score(testing_y, predictions))
