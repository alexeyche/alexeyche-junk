#!/usr/bin/env python
import numpy as np
import scipy.stats

# Generate correlated series of binary data

nobs = 150 # Lenght of the series
x = np.zeros((nobs))
# Create autocorrelated data
for i in range(1,nobs):
    x[i] = .25 * x[i-1] + np.random.randn()

# Do a logit transform, converts x to 0-1 interval
p = 1./(1+np.exp(-x))

# Define a Bernoulli distribution using p


# Call the random number generator
tseries1 = np.array( [scipy.stats.bernoulli.rvs( p[i]) for i in xrange(nobs)] )
tseries2 = np.array( [scipy.stats.bernoulli.rvs( p[i]) for i in xrange(nobs)] )
tseries3 = np.array( [scipy.stats.bernoulli.rvs( p[i]) for i in xrange(nobs)] )
