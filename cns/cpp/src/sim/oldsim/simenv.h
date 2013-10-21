#ifndef SIMENV_H
#define SIMENV_H

#include <sim/main/basic_elements.h>
#include <sim/main/neurons.h>
#include <sim/main/poisson.h>
#include <sim/main/connection.h>
#include <sim/main/stat_collector.h>
#include <sim/main/synapse.h>

class SimEnv {
    public: 
        SimEnv() {}
        
        Neurons* addNeuronGroup(NeuronGroupOptions opts);
        StatCollector* addStatCollector(vec &v);
        Poisson* addPoissonElem(double mHerz, double long_ms, double Iout_value);
        Synapse* addSynapse(Neurons *pre, Neurons *post,  SynapticGroupOptions &sgo);
        template <typename TConnection>
        TConnection* connect(SimElemCommon* pre, SimElemCommon* post) {
            TConnection* c = new TConnection(pre, post);
            connections.push_back(c);
            return c;   
        }       

        void runSimulation(SimOptions so); 
        void prepareSimulation(SimOptions so);
        
        std::vector<SimBasic*> elements;
        std::vector<Connection*> connections;        
};

#endif
