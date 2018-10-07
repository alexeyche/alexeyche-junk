
import numpy as np
import logging


from sklearn.metrics import confusion_matrix
from sklearn.metrics import accuracy_score
from sklearn.metrics import classification_report
from sklearn.metrics import roc_auc_score
from sklearn.metrics import f1_score


logger = logging.getLogger("metric")


class Metric(object):    
    def __call__(self, y_true, y_score):
        raise NotImplementedError


class FunWrapMetric(Metric):
    def __init__(self, fun):
        self.fun = fun

    def __call__(self, y_true, y_score):
        return self.fun(y_true, y_score)

class AUCMetric(FunWrapMetric):
    def __init__(self): super(AUCMetric, self).__init__(roc_auc_score)

class AccuracyMetric(FunWrapMetric):
    def __init__(self): super(AccuracyMetric, self).__init__(accuracy_score)

class F1Metric(FunWrapMetric):
    def __init__(self): super(F1Metric, self).__init__(f1_score)
