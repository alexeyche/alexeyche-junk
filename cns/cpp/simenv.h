#ifndef SIMENV_H
#define SIMENV_H

#include "core.h"
#include "neurons.h"
#include "simelem.h"

class SimEnv {
    public: 
        SimEnv(double tau_v = 0.25) : tau(tau_v) { }
        void addNeuronGroup(int n);
        
        void runSimulation(double time_s); 
        
        
        std::vector<SimElem*> elements;
        double tau;
};

#endif
