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
        
        Neurons* addNeuronGroup(NeuronGroupOptions opts);
        VoltMeter* addVoltMeter(Neurons *el);
        Poisson* addPoissonElem(double mHerz, double long_ms, double Iout_value);
        Connection* connect(SimElemCommon* pre, SimElemCommon* post);
        void runSimulation(SimOptions so); 
        void prepareSimulation(SimOptions so);
        
        std::vector<SimBasic*> elements;
        std::vector<Connection*> connections;        
};

#endif
