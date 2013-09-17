This readme and the code were contributed by Timothee Masquelier
timothee.masquelier@alum.mit.edu
Sep 2011

This code was used in:

Gilson M*, Masquelier T* and Hugues E. STDP allows fast rate-modulated
coding with Poisson-like spike trains. PLoS Computational Biology
* = equal contribution

Feel free to use/modify but please cite us if appropriate.

We use the Brian simulator described in: Goodman D, Brette R (2008)
Brian: a simulator for spiking neural networks in python. Front
Neuroinformatics 2:5.  and available at:
http://www.briansimulator.org/

This code has been tested with:
   - Brian 1.1.2
   - Python 2.5 and 2.6
   - Windows XP and Linux

Note that we did not use the STDP class provided in Brian. For
flexibility issues we preferred to code STDP manually (using embedded
C code for faster computations)

The base implementation corresponds to the 'all-to-all' additive STDP
of: Song S, Miller K, Abbott L (2000) Competitive hebbian learning
through spike-timing-dependent synaptic plasticity. Nat Neurosci
A 'nearest-spike' implementation is also provided.
Use mu > 0 to interpolate with multiplicative STDP as in:

Gutig R, Aharonov R, Rotter S, Sompolinsky H (2003) Learning input
correlations through nonlinear temporally asymmetric hebbian
plasticity. J Neurosci

Rate-based homeostatic terms w_in and w_out are also implemented as
in: Kempter R, Gerstner W, van Hemmen JL (1999) Hebbian learning and
spiking neurons. Phys Rev E

Main file: main.py (should be called like that "python -i main.py")
Calls init.py to set the parameters (see comments there). The current
configuration corresponds to the baseline simulation (Poisson neuron,
additive STDP)
Instantiate all the needed neurons: "mirror" layer (copy of the input
SpikeGeneratorGroup usefull for implementation issues), output layer.
connect them, and finally runs the Brian simulator.
All the data files are read and dumped in ./data

Spikes from the input layer are read from ./data/spikeList.###.###.mat
files (first number is the random seed, second number is the period
number).
Each file contain a (n,2) array called sl : neuron (start from 0), and
spike time in s
These files need to be generated before the simulation.

We also provided the Matlab code we used to do so.  Just launch
generateSpikeTrain.m with matlab The parameters (with comments) are
gathered in param.m The current configuration corresponds to "Model R"
in the paper (see Fig. 1R).

Note that with the current parameters generateSpikeTrain.m generates
1000s long spike trains.
To reproduce the results of the paper (T=2000s) you need to launch
twice main.py (by default the final weights are dumped, and the next
simulation use them as initial values)

To take advantage of Python vectorization we can simulate multiple
output neuron in parallel, with different gmax and LTD/LTP ratios
(this is useful for parameter exploration)
nG is the number of gmax values
nR is number of ratio LTD/LTP
M = nG*nR is the number of output neurons, numbered like that [
(r_0,g_0)...(r_0,g_nG),(r_1,g_0)...(r_1,g_nG),...,(r_nR,g_0)...(r_nR,g_nG)]
Note that instead of LTD/LTP ratios you can also explore various
w_in/w_out ratios.
For plotting it may be useful to provide a realValuedPattern.XXX.mat
file with the repeating pattern activation levels in a (1,n) array
called realValuedPattern
For plotting and mutual info computation it may be useful to provide a
patternPeriod.XXX.mat file with a (n,2) array (start,end) in s called
patternPeriod

--------------------
Needed Python files:
--------------------
main.py                 main script
init.py                 contains all the parameters
customrefractoriness.py Brian file to handle both a refractory period
                        and a user-defined reset function

--------------------
Useful Python files:
--------------------
analyze.py     plotting
saveWeight.py  dumps weights in ./data/weight.###.mat
savePot.py     dumps membrane potentials in ./data/potential.###.matand
               and output spikes in ./data/outputSpike.###.mat
saveCurrent.py dumps currents in ./data/current.###.mat

--------------------
Useful Matlab files:
--------------------
convergence.m : to be launched after the simulation. Plots the
                convergence index, as a function of time (see Fig 3Conv)

--------------------------------------------------
Under developpement (provided with no garanty...):
--------------------------------------------------
mutualInfo.py  (under development): computes, plots and dumps the
               mutual info between presence of the stimulus and
               postsynaptic responses
toMatlab.py    (under development): exports data in a mat file for later
               use with Matlab
pickleAll.py   (underdevelopment): dump all variables
unpickleAll.py (underdevelopment): load all (previously dumped) variables
restore.py     (underdevelopment): restore the final state of a previous
               simulation
