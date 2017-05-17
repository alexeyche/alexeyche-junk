
import numpy as np

class Optimization(object):
	def init(self, *shapes):
		raise NotImplementedError

	def update(self, *dparams):
		raise NotImplementedError



class MomentumOpt(Optimization):
	def __init__(self, learning_rates, gamma):
		self.gamma = gamma
		# self.learning_rates = [ lr * (1.0 - gamma) for lr in learning_rates]
		self.learning_rates = learning_rates
		
		self.moments = []

	def init(self, *params):
		self.moments = []

		for param in params:
			self.moments.append(np.zeros(param.shape))

	def update(self, *param_and_dparam):
		assert len(self.moments) == len(param_and_dparam)

		for pi, (param, dparam) in enumerate(param_and_dparam):
			self.moments[pi] = self.moments[pi] * self.gamma + self.learning_rates[pi] * dparam
			yield param - self.moments[pi]

class NesterovMomentumOpt(Optimization):
	def __init__(self, learning_rates, gamma):
		self.gamma = gamma
		self.learning_rates = learning_rates
		
		self.moments = []
		self.prev_moments = []
		
	def init(self, *params):
		self.moments = []
		self.prev_moments = []

		for param in params:
			self.moments.append(np.zeros(param.shape))

	def update(self, *param_and_dparam):
		assert len(self.moments) == len(param_and_dparam)
		self.prev_moments = [m.copy() for m in self.moments]

		for pi, (param, dparam) in enumerate(param_and_dparam):
			self.moments[pi] = self.moments[pi] * self.gamma - self.learning_rates[pi] * dparam
			yield param - self.gamma * self.prev_moments[pi] + (1.0 + self.gamma) * self.moments[pi]

class SGDOpt(Optimization):
	def __init__(self, parameters, learning_rates):
		self.learning_rates = learning_rates
		self.parameters = parameters
	
	def update(self, *dparams):
		for pi, dparam in enumerate(dparams):
			self.parameters[pi] -=self.learning_rates[pi] * dparam


class AdagradOpt(Optimization):
	def __init__(self, learning_rates, eps=1e-05):
		self.learning_rates = learning_rates
		self.eps = eps
		self.cache = []

	def init(self, *params):
		self.cache = [np.zeros(p.shape) for p in params]
		
	def update(self, *param_and_dparam):
		assert len(self.cache) == len(param_and_dparam)

		self.cache = [ dp_cache + np.square(dp) for dp_cache, (_, dp) in zip(self.cache, param_and_dparam)]
		for pi, (param, dparam) in enumerate(param_and_dparam):
			yield param - self.learning_rates[pi] * dparam / (np.sqrt(self.cache[pi]) + self.eps)


class RMSPropOpt(Optimization):
	def __init__(self, learning_rates, decay_rate=0.9, eps=1e-05):
		self.learning_rates = learning_rates
		self.eps = eps
		self.decay_rate = decay_rate
		self.cache = []

	def init(self, *params):
		self.cache = [np.zeros(p.shape) for p in params]
		
	def update(self, *param_and_dparam):
		assert len(self.cache) == len(param_and_dparam)

		for pi, (param, dparam) in enumerate(param_and_dparam):
			self.cache[pi] = self.cache[pi] * self.decay_rate + (1.0 - self.decay_rate) * np.square(dparam)

			yield param - self.learning_rates[pi] * dparam / (np.sqrt(self.cache[pi]) + self.eps)



class AdamOpt(Optimization):
	def __init__(self, parameters, learning_rates, beta1=0.9, beta2=0.999, eps=1e-05):
		assert len(parameters) == len(learning_rates)
		
		self.parameters = parameters
		self.learning_rates = learning_rates
		
		self.eps = eps
		
		self.beta1 = beta1
		self.beta2 = beta2

		self.m = [np.zeros(p.shape) for p in self.parameters]
		self.v = [np.zeros(p.shape) for p in self.parameters]

	def update(self, *dparams):
		assert len(self.m) == len(dparams) and len(self.v) == len(self.parameters)
		
		for pi, dparam  in enumerate(dparams):
			self.m[pi] = self.m[pi] * self.beta1  + (1.0 - self.beta1) * dparam
			self.v[pi] = self.v[pi] * self.beta2  + (1.0 - self.beta2) * np.square(dparam)
			
			self.parameters[pi] -= self.learning_rates[pi] * self.m[pi] / (np.sqrt(self.v[pi]) + self.eps)












