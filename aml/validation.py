import numpy as np
import logging

import pandas as pd
from tabulate import tabulate

from metric import *
from operation import Operation
from feature_pool import FeaturePool
from model import Model

from sklearn.metrics import confusion_matrix
from sklearn.metrics import classification_report
from sklearn.utils.multiclass import unique_labels




logger = logging.getLogger("validation")


def pretty_table(header, data, columns, rows):
    return pretty_df(
        header,
        pd.DataFrame(data, index=rows, columns=columns)
    )


def pretty_df(header, df):
    df_s = tabulate(
        df,
        headers='keys',
        tablefmt='psql'
    )

    return (
        "\t" + header + "\n" +
        "\t" + "\t".join(df_s.splitlines(True))
    )


class Validation(Operation):
    def __init__(
        self,
        metrics=(),
    ):
        self.metrics = metrics

    def do(self, mo):
        _, test_x, test_y = mo.fp.test_arrays()
        return Model.Output(
            [
                self.validate(m, test_x, test_y)
                for m in mo.models
            ],
            mo.fp
        )


class VClassificationReport(Validation):
    def __init__(self):
        super(VClassificationReport, self).__init__(
            metrics=(
                AUCMetric(),
                F1Metric(),
                AccuracyMetric()
            )
        )

    def validate(self, model, test_x, test_y):
        pred_test_y = model.predict(test_x)
        cr = classification_report(test_y, pred_test_y, output_dict=True)
        cm = confusion_matrix(test_y, pred_test_y)

        mvals = []
        mnames = []
        for metric in self.metrics:
            v = metric(test_y, pred_test_y)
            mvals.append(v)
            mnames.append(metric.name())

        labs = unique_labels(test_y)

        report = "\n"
        report += "\tClassificationReport for `{}`\n".format(model.name)
        report += "\n"
        report += pretty_df("Report", pd.DataFrame(cr).transpose())
        report += "\n\n"
        report += pretty_table("Metric results", mvals, ["Metric"], mnames)
        report += "\n\n"
        report += pretty_table("Confusion matrix", cm, labs, labs)

        logger.info("VClassificationReport: \n{}".format(report))
        return model
