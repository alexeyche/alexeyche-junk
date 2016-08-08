#!/usr/bin/env python

import logging
import sys
import numpy as np
import os
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from util import setup_logging

from conv_model import ConvModel
from env import current as env


setup_logging(logging.getLogger())


data_source = []
for f in sorted(os.listdir(env.dataset())):
    if f.endswith(".wav"):
        data_source.append(env.dataset(f))



cm = ConvModel(
	batch_size = 30000,
	filter_len = 150,
	filters_num = 100,
	target_sr = 3000,
	gamma = 1e-03,
	strides = 8,
	avg_window = 5,
	lrate = 1e-04
)



sess = tf.Session()

dataset = cm.form_dataset(data_source, proportion = 0.1)

cm.train(sess, dataset, 10000)
cm.evaluate_and_save(sess, dataset)
cm.serialize(sess)
