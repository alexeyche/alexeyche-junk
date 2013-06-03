#ifndef SIMENV_H
#define SIMENV_H

#include "basic_elements.h"

#include "neurons.h"
#include "poisson.h"
#include "connection.h"
#include "voltmeter.h"



class SimEnv {
    public: 
        SimEnv() {}
        
        Neurons* addNeuronGroup(int n);
        VoltMeter* addVoltMeter(Neurons *el);
        Poisson* addPoissonGroup(int n, double mHerz);
        Connection* connect(SimElemCommon* pre, SimElemCommon* post);
        void runSimulation(SimOptions so); 
        void prepareSimulation(SimOptions so);
        
        std::vector<SimBasic*> elements;
        std::vector<Connection*> connections;        
};

#endif