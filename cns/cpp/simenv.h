#ifndef SIMENV_H
#define SIMENV_H

#include "basic_elements.h"

#include "neurons.h"
#include "poisson.h"
#include "connection.h"
#include "voltmeter.h"

class SimEnv {
    public: 
        SimEnv(double tau_v = 0.25) : tau(tau_v) { }
        
        Neurons* addNeuronGroup(int n);
        VoltMeter* addVoltMeter(Neurons *el);
        Poisson* addPoissonGroup(int n, double mHerz);
        Connection* connect(SimElemCommon* pre, SimElemCommon* post);
        void runSimulation(double time_s); 
        
        
        std::vector<SimBasic*> elements;
        std::vector<Connection*> connections;
        double tau;
};

#endif
