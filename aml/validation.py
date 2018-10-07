import numpy as np
import logging

from operation import Operation

logger = logging.getLogger("validation")


class Validation(Operation):
    def __init__(
        self, 
        models=(), 
        metrics=(),
    ):
        self.models = models
        self.metrics = metrics



class VClassificationReport(Validation):
    pass