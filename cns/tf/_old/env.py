
import os
from os.path import join as pj

def make_dir(*args):
	if not os.path.exists(pj(*args)):
		os.makedirs(pj(*args))
	return pj(*args)


class Env(object):
	ROOT_PATH = pj(os.environ["HOME"], "Music", "ml")

	def __init__(self, dataset):
		self._res_dir = make_dir(self.ROOT_PATH, dataset, "result")
		self._dataset_dir = make_dir(self.ROOT_PATH, dataset, "dataset")
		self._run_dir = make_dir(self.ROOT_PATH, dataset, "run")


	def result(self, *args):
		if len(args)>0:
			return pj(self._res_dir, *args)
		return self._res_dir

	def dataset(self, *args):
		if len(args)>0:
			return pj(self._dataset_dir, *args)
		return self._dataset_dir

	def run(self, *args):
		if len(args)>0:
			return pj(self._run_dir, *args)
		return self._run_dir
