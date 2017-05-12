
import numpy as np

import xgboost as xgb

import sys
from datasets import get_toy_data

x_values, y_values = get_toy_data(seed=2)

n_train = (4 * x_values.shape[0]/5)
n_valid = (1 * x_values.shape[0]/5)
        
x_train = x_values[:n_train]
x_test = x_values[n_train:(n_train+n_valid)]

y_train = y_values[:n_train]
y_test = y_values[n_train:(n_train+n_valid)]

xg_train = xgb.DMatrix(x_train, label=y_train)
xg_test = xgb.DMatrix(x_test, label=y_test)

param = {}
# use softmax multi-class classification
param['objective'] = 'multi:softmax'
# param['objective'] = 'binary:logistic'
# scale weight of positive examples
param['eta'] = 0.1
param['max_depth'] = 6
param['silent'] = True
param['nthread'] = 4
param['num_class'] = len(np.unique(y_values))
param['booster'] = "gbtree"
param['gamma'] = 0.1
param['lambda'] = 5.0
param['min_child_weight'] = 1
# param['eval_metric'] = 'rmse'

watchlist = [ (xg_train,'train'), (xg_test, 'test') ]

num_round = 70

bst = xgb.train(param, xg_train, num_round, watchlist )

pred = bst.predict(xg_test)

print 'predicting, classification error=%f' % (
	sum( int(pred[i]) != y_test[i] for i in range(len(y_test))) / float(len(y_test)) 
)
