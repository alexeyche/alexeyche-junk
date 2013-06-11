# -*- coding: utf-8 -*-


'''
Paper:Peter beim Graben and Serafim rodrigues
A Biophysical observation model for field potentials of networks of leaky-integrate-and-fire neurons.
Front. Comput. Neurosci, 04 January 2013 | doi: 10.3389/fncom.2012.00100

*******************************
This python code that runs under the Brian simulator.


Note for developers:
Note1 : As it stands, the code is not effiecient (fast) as it does not use the facilties vector processing and uses a lot of for-loops which is not efficient.
So it can be improved.
Note 2: Periodic thalamic input is not yet implemented.

*****************************

1) This is a network of 5000 neurons, 80% of which excitatory, and 
   20% inhibitory.
2) The network is randomy connected (between pairs) with connection 
   probability = 0.2.
3) Both Excitatory and Inhibitory neurons are described via LIF model.
4) The currents are double exponetial, but the excitatory currents
   can recieve external noise.

'''
from brian import *
from statlib import stats as st
import pylab
##import matplotlib.pyplot as plt
from math import *
from sys import *
#import numpy


#General parameters
setup_duration = 1750*ms # initial run to settle behaviour
duration = 250*ms  # Run time for recording
#duration = 2000*ms
defaultclock.dt = 0.01 * ms


#----------------------------- GLOBAL Parameters ----------------------------------------------------------
tau_mE = 20*ms    # membrane time constant for excitatory [pyramidal] neurons
tau_mI = 10*ms    # membrane time constant for inhibitory neurons
tau_rG_I = 0.25*ms  # rise time of GABA synaptic current (interneurons)
tau_dG_I = 5*ms     # decay time of GABA synaptic current (interneurons)
tau_rG_E = 0.25*ms  # rise time of GABA synaptic current (pyramidal cells)
tau_dG_E = 5*ms     # decay time of GABA synaptic current (pyramidal cells)
tau_rA_I = 0.2*ms # rise time of AMPA synpatic current (on interneurons)
tau_dA_I = 1*ms   # decay time of AMPA synpatic current (on interneurons)
tau_rA_E = 0.4*ms # rise time of AMPA synpatic current (pyramidal [pyr] cells)
tau_dA_E = 2*ms   # decay time of AMPA synpatic current (pyramiday[pyr] cells)
tau_rp_E = 2*ms   # refractory period for Pyramidal cells
tau_rp_I = 1*ms   # refractory period of Inhibitory cells
Vt = 18*mV        # spike threshold
Vreset = 11*mV    # Reset value
El = 0 * mV       # resting potential

# Latency of Post Synaptic Potential
tau_EL = 1*ms # For exctiatory neurons
tau_IL = 1*ms # For inhibitory neurons
tau_TL = 1*ms # For thalamic input


J_GABA_E = 1.7*mV  # Gaba synaptic efficacy on pyramidal cells
J_GABA_I = 2.7*mV  # Gaba synaptic efficacy on interneurons
J_AMPA_E = 0.42*mV # Ampa synaptic efficacy on pyramidal cells
J_AMPA_I = 0.7*mV  # Ampa synaptic efficacy om Interneurons
J_Ext_E = 0.55*mV  # External/Thalamic synpatic efficacy on pyramidal cells
J_Ext_I = 0.95*mV  # External/Thalamic synaptic efficacy on interneurons



Number_Of_CorticalNeurons = 5000 # Total number of Cortiucal Neurons
##P_Interneurons = 0.2              # 20% Percent of Interneurons
##P_Excitatory = 1 - P_Interneurons # Percentage of Excitatory neurons
##N_Interneurons = int(P_Interneurons*Number_Of_CorticalNeurons)
##N_Excitatory = int(P_Excitatory*Number_Of_CorticalNeurons)

N_Interneurons = 1000
N_Excitatory = 4000
Number_Of_ThalamicNeurons = 5000 # Thalamic/external Input (every neuron will recieve a different external realisation)




#-----------------------------Parameters  ------------------------------------------------------

r_i = 1.7e9 
Rho_Cytoplasm = 2
Rho_Extracellular = 3.33  
Rho_Membrane_Hillock  = 0.005 
Length_Dendrite = 2e-4 
Radius_Dendrite = 7e-6
Length_Axon_Hillock = 2e-5
Radius_Axon_Hillock = 5e-7
Area_Axon_Hillock = (2*pi*(Radius_Axon_Hillock)**2 + 2*pi*Radius_Axon_Hillock*Length_Axon_Hillock)
Area_Extracellular = ((12*sqrt(3) - 3*pi)*(Radius_Dendrite)**2)
R_A = Rho_Cytoplasm*(Length_Dendrite/2)/(pi*Radius_Dendrite**2) 
R_B = Rho_Cytoplasm*(Length_Dendrite/2)/(pi*Radius_Dendrite**2)
R_C = Rho_Extracellular*(Length_Dendrite/2)/Area_Extracellular
R_D = Rho_Extracellular*(Length_Dendrite/2)/Area_Extracellular
R_M = Rho_Membrane_Hillock/Area_Axon_Hillock
print "R_A", R_A
print "R_B", R_B
print "R_C", R_C
print "R_D", R_D
print "R_M", R_M



#----------- Setup Cortical Model ----------------------------
ENeuronModel = Equations('''
        dV/dt  = (-(V-El) + IA - IG)/tau_mE : volt
        dIA/dt = (-IA + XA)/tau_dA_E         : volt
        dXA/dt = -XA/tau_rA_E                : volt
        dIG/dt = (-IG + XG)/tau_dG_E         : volt
        dXG/dt = -XG/tau_rG_E                : volt
        dIA2/dt = (-IA2 + XA2)/tau_dA_E      : volt
        dXA2/dt = -XA2/tau_rA_E              : volt
        dIG2/dt = (-IG2 + XG2)/tau_dG_E      : volt
        dXG2/dt = -XG2/tau_rG_E              : volt
        LFP = abs(IA) + abs(IG)              : volt
        DFP = IA2 + IG2 + PSI*V              : volt
        PSI                                  : 1
        ''')

INeuronModel = Equations('''
        dV/dt  = (-(V-El) + IA - IG)/tau_mI : volt
        dIA/dt = (-IA + XA)/tau_dA_I         : volt
        dXA/dt = -XA/tau_rA_I                : volt
        dIG/dt = (-IG + XG)/tau_dG_I         : volt
        dXG/dt = -XG/tau_rG_I                : volt
        LFP = abs(IA) + abs(IG)              : volt
        ''')
Ge = NeuronGroup(N_Excitatory, model=ENeuronModel, reset=Vreset, threshold=Vt, refractory=tau_rp_E) # Excitatory neurons
Gi = NeuronGroup(N_Interneurons, model=INeuronModel, reset=Vreset, threshold=Vt, refractory=tau_rp_I) # Inhibitory neurons


'''
And in order to start the network off in a somewhat
more realistic state, we initialise the membrane
potentials uniformly randomly between the reset and
the threshold.
'''
Ge.V = Vreset + (Vt - Vreset) * rand(len(Ge))
Gi.V = Vreset + (Vt - Vreset) * rand(len(Gi))
Ge.IA = zeros(len(Ge))
Gi.IA = zeros(len(Gi))
Ge.XA = zeros(len(Ge))
Gi.XA = zeros(len(Gi))
Ge.IG = zeros(len(Ge))
Gi.IG = zeros(len(Gi))
Ge.XG = zeros(len(Ge))
Gi.XG = zeros(len(Gi))
Ge.LFP = zeros(len(Ge))
Gi.LFP = zeros(len(Gi))
Ge.DFP = zeros(len(Ge))

       
#------------------------- Setup Thalamic Input ----------------------------
''' 
The Thalamic input is essential noise. The idea is to account for both cortical Heterogeneity and Spontaneous actvity.
For this reason we have two levels of noise. The first one is a an Ornstein-Uhlenbeck Process + (either constant or periodic signal)
with rate(t). The second level is time varying Inhomogenous Poisson process,with rate(t)
'''
# Ornstein-Uhlenbeck Process Parameters
tau_n = 16 * ms # Auto-Correlation time constant of the noise
st_dev = 400*Hz # Standard deviation [spikes/ms]

# Periodic signal parameters
##v_0 = 2400*Hz    # Base line of the periodic signal
##a = 400*Hz      # Amplitude of the periodic signal
##omega = 4 *Hz   # Angualr Frequency

v_0 = 1600*Hz
a = 400*Hz
omega = 10 *Hz   # Angualr Frequency

# Thalamic model
# This version is constant signal + noise
##OU_Constant = Equations(''' 
##dn/dt = -n/tau_n + st_dev*(2./tau_n)**.5*xi :Hz 
##rate = v_0 + n :Hz''')

# This version is periodic signal + noise
OU_Periodic = Equations(''' 
dn/dt = -n/tau_n + st_dev*(2./tau_n)**.5*xi :Hz 
signal = v_0 + a*sin(2*pi*omega*t) :Hz
rate = signal + n : Hz
''')


# This group does not produce spikes, and when negative rate, it will be considred as 0
# by the Poisson threshold on the seocond noise level 
#OU = NeuronGroup(1, model=OU_Constant)

OU = NeuronGroup(1, model=OU_Periodic)

# Second level noise
# Run N_Neurons realisations of inhomogenous Poisson process with rate, rate(t) given
# by the Ornstein_Uhlenbeck process.
Thalamic_IP = NeuronGroup(Number_Of_ThalamicNeurons, model='P : Hz', threshold=PoissonThreshold(state='P')) # inhomogenous Poisson
#  IP.P = rand(N_Neurons) no need for this...
Thalamic_IP.P = linked_var(OU, 'rate')




#------------------ Define Network Connections and Weights -----------------
# Cortical Connections
# Note we make conversion from mazzoni to Peters model according to:
# J_{i,j}= w_{i,j}*v, where v=1mV and w_{i,j} is the true weight
# Example: J_GABA_I = w_{i,j}*v

#------ Inhibitory neurons recieving inputs ---------


Wii = (tau_mI*J_GABA_I)/tau_rG_I # Synaptic Weight
Cii = Connection(Gi, Gi, 'XG', sparseness=0.2, weight=Wii, delay=tau_IL) # Inhibiotry-Inhibitory

Wei = (tau_mI*J_AMPA_I)/tau_rA_I # Synaptic weight
Cei = Connection(Ge, Gi, 'XA', sparseness=0.2, weight=Wei, delay=tau_EL) # Excitatory-Inhibitory


#------ Excitatory neurons recieving inputs --------
Wie = (tau_mE*J_GABA_E)/tau_rG_E # Synaptic Weight
Cie = Connection(Gi, Ge, 'XG', sparseness=0.2, weight=Wie, delay=tau_IL) # Inhibitory-Excitatory

Wee = (tau_mE*J_AMPA_E)/tau_rA_E # synaptic Weight
Cee = Connection(Ge, Ge, 'XA', sparseness=0.2, weight=Wee, delay=tau_EL) # Excitatory-Excitatory


#------------------ Define External Thalamic Input -----------------
# External Thalamic Input
Wti = (tau_mI*J_Ext_I)/tau_rA_I # synaptic Weight
Cti = Connection(Thalamic_IP, Gi, 'XA', delay=tau_TL) # Thalamic-Inhibitory
Cti.connect_one_to_one(Thalamic_IP[N_Excitatory:Number_Of_ThalamicNeurons], Gi, weight=Wti)


Wte = (tau_mE*J_Ext_E)/tau_rA_E # synaptic Weight
Cte = Connection(Thalamic_IP, Ge, 'XA',delay=tau_TL) # Thalamic-Exictatory
Cte.connect_one_to_one(Thalamic_IP[0:N_Excitatory], Ge, weight=Wte) 

#---------------------------------------------------------------------------------







alpha_list=[0.32*1e-9]*len(Ge) 
gI_list = [0]*len(Ge)
gE_list = [0]*len(Ge)
for i in range(len(Ge)):
  for j in range(len(Ge)):
    if Cee[j,i] != 0 :
       alpha_list[i] = alpha_list[i] + 0.25*1e-9 # 0.25*nS + 0.32*nS (cortical + thalamic)
#    print "Cee[j, i]", Cee[j, i]
       
  for k in range(len(Gi)):
     if Cie[k,i] !=0:
       gI_list[i] = gI_list[i] + 1*1e-9 # gaba = 1nS



# Evaluate other parameters
for i in range(len(Ge)):
   gE_list[i] = alpha_list[i]/(1 - (R_A +  R_D)*alpha_list[i])
   




We_tilda = [0]*len(Ge)
Wi_tilda = [0]*len(Ge)
for i in range(len(Ge)):
    Beta_tau_i = gE_list[i]*(R_B + R_C)/(r_i*(1 + gE_list[i]*(R_A+R_D) + (R_B+R_C)*(gE_list[i] - gI_list[i]*(1 + gE_list[i]*(R_A+R_D)))))
    gamma_i =  (gE_list[i]*(R_M + R_B + R_C))/(R_M*(1 + gE_list[i]*(R_A + R_D)))
##    We_tilda[i] = R_D*(J_AMPA_E/mV)*(1/r_i - Beta_tau_i)
##    Wi_tilda[i] = R_D*(J_GABA_E/mV)*Beta_tau_i
    We_tilda[i] = R_D*(J_AMPA_E)*(1/r_i - Beta_tau_i)
    Wi_tilda[i] = R_D*(J_GABA_E)*Beta_tau_i
    Ge[i].PSI = R_D*(Beta_tau_i - gamma_i)
    if i==1:
      print "Beta over tau 0:", Beta_tau_i
      print "gamma_0:", gamma_i
      print "We_tilda[0]:", We_tilda[0] 
      print "Wi_tilda[0]:", Wi_tilda[0]
      print "PSI", Ge[0].PSI



#------ Now establish connections with \tilda{w}------
# Inhibitory-Excitatory
Cie2 = Connection(Gi, Ge, 'XG2', delay=tau_IL) 

for i in range(len(Ge)):
   for j in range(len(Gi)):
     if Cie[j, i] != 0:
        Cie2[j, i] =  (tau_mE*Wi_tilda[j])/tau_rG_E

print "Wie:", (tau_mE*J_GABA_E)/tau_rG_E
print "Cie2[0, i]", (tau_mE*Wi_tilda[0])/tau_rG_E

# Excitatory-Excitatory
Cee2 = Connection(Ge, Ge, 'XA2', delay=tau_EL) 

for i in range(len(Ge)):
   for j in range(len(Ge)):
     if Cee[j, i] != 0:
        Cee2[j, i] = (tau_mE*We_tilda[j])/tau_rA_E


print "Wee:", (tau_mE*J_AMPA_E)/tau_rA_E 
print "Cee2[0, i]", (tau_mE*We_tilda[0])/tau_rA_E


# Thalamic-Exictatory
Cte2 = Connection(Thalamic_IP, Ge, 'XA2',delay=tau_TL)
Cte2.connect_one_to_one(Thalamic_IP[0:N_Excitatory], Ge, weight=Wte)
for i in range(len(Ge)):
   for j in range(len(Thalamic_IP[0:N_Excitatory])):
        Cte2[j, i] = (tau_mE*We_tilda[j])/tau_rA_E


#-------------------------------------------------------------------





#--------------------------------Running and Printing ----------------------------------------------------------
# We do an initial run to settle behaviour before recording
run(setup_duration)


#------------------ Setup the main Observables --------------------------
'''
Set up some monitors as usual. The line ``record=0`` in the :class:`StateMonitor`
declarations indicates that we only want to record the activity of
neuron 0. This saves time and memory.
'''
ThalamicRate = StateMonitor(Thalamic_IP, 'P', record=0, timestep=10)
TS = SpikeMonitor(Thalamic_IP) # Thalamic Spikes

# Record Spikes
Ge_S = SpikeMonitor(Ge[0:200])
Gi_S = SpikeMonitor(Gi[0:200])

# Record every neurons LFPs and DFPs

##LFP_e = StateMonitor(Ge, 'LFP', record=True)
##DFP_e = StateMonitor(Ge, 'DFP', record=True)
##IA2_e = StateMonitor(Ge,'IA2', record=True)
##IG2_e = StateMonitor(Ge,'IG2', record=True)
##IA_e = StateMonitor(Ge,'IA', record=True)
##IG_e = StateMonitor(Ge,'IG', record=True)
##Membrane = StateMonitor(Ge,'V', record=True)


LFP_e = StateMonitor(Ge, 'LFP', timestep=10)
DFP_e = StateMonitor(Ge, 'DFP', timestep=10)
IA2_e = StateMonitor(Ge,'IA2', timestep=10)
IG2_e = StateMonitor(Ge,'IG2', timestep=10)
IA_e = StateMonitor(Ge,'IA', timestep=10)
IG_e = StateMonitor(Ge,'IG', timestep=10)
Membrane = StateMonitor(Ge,'V', timestep=10)



# Record the population rate/so we can compare with LFP
rate_i = PopulationRateMonitor(Gi, bin=1*ms)
rate_e = PopulationRateMonitor(Ge, bin=1*ms)



'''
Now we run.
'''
run(duration)
'''



And finally we plot the results. Just for fun, we do a rather more
complicated plot than we've been doing so far, with three subplots.
The upper one is the raster plot of the whole network, and the
lower two are the values of ``V`` (on the left) and ``ge`` and ``gi`` (on the
right) for the neuron we recorded from. See the PyLab documentation
for an explanation of the plotting functions, but note that the
:func:`raster_plot` keyword ``newfigure=False`` instructs the (Brian) function
:func:`raster_plot` not to create a new figure (so that it can be placed
as a subplot of a larger figure). 
'''


# Constant input signal rate
signal = ones(len(ThalamicRate[0])) * v_0


total_LFP = []
mean_LFP = []
for lfp in xrange(len(LFP_e.times)):
    total_LFP.append(sum(LFP_e[:,lfp]))
    mean_LFP.append(float(sum(LFP_e[:,lfp]))/ N_Excitatory)



total_DFP = []
mean_DFP = []
# mean2_DFP = []
for dfp in xrange(len(DFP_e.times)):
    total_DFP.append(sum(DFP_e[:,dfp]))
    mean_DFP.append(float(sum(DFP_e[:,dfp]))/N_Excitatory)
#     mean2_DFP.append(st.mean(DFP_e[:,dfp]))



# Mean membrane
mean_membrane = []
for i_v in xrange(len(Membrane.times)):
    mean_membrane.append(st.mean(Membrane[:,i_v]))


# Convert volt to mili-volt
mean_membrane[:] = [x*1e3 for x in mean_membrane]
#total_LFP[:] = [x*1e3 for x in total_LFP]
mean_LFP[:] = [x*1e3 for x in mean_LFP]
#total_DFP[:] = [x*1e3 for x in total_DFP]
mean_DFP[:] = [x*1e3 for x in mean_DFP]


#------------------- Various Plots --------------------- 
pylab.figure(1)
#pylab.yticks((0,0.5, 1))
plot(ThalamicRate.times/ms, ThalamicRate[0]/1000, 'k')
pylab.plot(ThalamicRate.times/ms, signal/1000, 'g')
filename="ThalamicRate_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')

pylab.figure(2)
#pylab.yticks((0,0.5, 1))
raster_plot(Ge_S)
filename="Excitatory_RasterPlot_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')

pylab.figure(3)
#pylab.yticks((0,0.5, 1))
pylab.plot(rate_e.times / ms, rate_e.rate, 'b')
filename="Excitatory_rate_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')

pylab.figure(4)
#pylab.yticks((0,0.5, 1))
pylab.plot(rate_i.times / ms, rate_i.rate, 'r')
filename="Inhibitory_rate_" + `v_0` + ".eps"             
pylab.savefig(filename, format='eps')

pylab.figure(5)
#pylab.yticks((0,0.5, 1))
pylab.plot(Membrane.times / ms, mean_membrane, 'g')
filename="mean_membrane_" + `v_0` + ".eps"     
pylab.savefig(filename, format='eps')

pylab.figure(6)
#pylab.yticks((0,0.5, 1))
pylab.plot(LFP_e.times / ms, total_LFP, 'm')
filename="total_LFP_" + `v_0` + ".eps" 
pylab.savefig(filename, format='eps')


pylab.figure(7)
#pylab.yticks((0,0.5, 1))
pylab.plot(LFP_e.times / ms, mean_LFP, 'm')
filename="mean_LFP_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')


pylab.figure(8)
#pylab.yticks((0,0.5, 1))
plot(DFP_e.times / ms, total_DFP, 'c')
filename="total_DFP_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')

pylab.figure(9)
#pylab.yticks((0,0.5, 1))
pylab.plot(DFP_e.times / ms, mean_DFP, 'c')
filename="mean_DFP_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')


#f=open("data", "w")
#numpy.savetxt(f, numpy.array([DFP_e.times, mean_DFP]).T)
#f.close()


#------------------- Ploting Power spectra --------------------- 

dt=0.0001 # 0.1ms = 0.0001 sec
#nextpow2=32768 # here 2000ms of length
nextpow2=4096 # only 250ms of length 

pylab.figure(10)
psd(total_LFP, nextpow2, 1/dt) 
filename="PowerSpectra_total_LFP_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')

pylab.figure(11)
psd(mean_LFP, nextpow2, 1/dt) 
filename="PowerSpectra_mean_LFP_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')


pylab.figure(12)
psd(total_DFP, nextpow2, 1/dt)
filename="PowerSpectra_total_DFP_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')

pylab.figure(13)
psd(mean_DFP, nextpow2, 1/dt)
filename="PowerSpectra_mean_DFP_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')


pylab.figure(14)
psd(mean_membrane, nextpow2, 1/dt)
filename="PowerSpectra_mean_membrane_" + `v_0` + ".eps"
pylab.savefig(filename, format='eps')

pylab.show()

##### --------------------------END --------------------------------
