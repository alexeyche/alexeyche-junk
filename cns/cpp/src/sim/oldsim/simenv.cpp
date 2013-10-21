
#include "simenv.h"
#include "axon.h"

Neurons* SimEnv::addNeuronGroup(NeuronGroupOptions opts) { 
    Neurons *elems = new Neurons(opts);
    elements.push_back((SimBasic*)elems);
    elements.push_back((SimBasic*)elems->axon);
    return elems;
}

Synapse* SimEnv::addSynapse(Neurons *pre, Neurons *post,  SynapticGroupOptions &sgo) { 
    Synapse *elems = new Synapse(pre, post, sgo);
    connections.push_back((Connection*)elems);
    return elems;
}

StatCollector* SimEnv::addStatCollector(vec &v) { 
    StatCollector *elems = new StatCollector(v);
    elements.push_back((SimBasic*)elems);
    return elems;
}

Poisson* SimEnv::addPoissonElem(double mHerz, double long_ms, double Iout_value) { 
    Poisson *elems = new Poisson(mHerz,long_ms,Iout_value);
    elements.push_back((SimBasic*)elems);
    return elems;
}

void SimEnv::prepareSimulation(SimOptions so) {
    for(size_t i=0; i<elements.size(); i++) {
            elements[i]->prepareMe(so);        
    }
    for(size_t i=0; i<connections.size(); i++) {
            connections[i]->prepareMe(so);        
    }
}

void SimEnv::runSimulation(SimOptions so) {
    this->prepareSimulation(so);
    Timer::Start("Iterations");
    for(double t=0; t < so.time_ms; t+=so.tau_ms) {        
        Log::Info << "1) Calculating elements (t:" << t << ")" << std::endl;
        for(size_t i=0; i<elements.size(); i++) {            
            Timer::Start("Element");
            Log::Info << "===| Calculating element #" << i << std::endl;
            elements[i]->computeMe(so.tau_ms);
            Timer::Stop("Element");        
        }
        Log::Info << "2) Calculating connections (t:" << t << ")" << std::endl;
        for(size_t i=0; i<connections.size(); i++) {            
            Timer::Start("Connection");
            Log::Info << "===| Calculating connection #" << i << std::endl;
            connections[i]->computeMe(so.tau_ms);        
            Timer::Stop("Connection");
        }
    }
    Timer::Start("Iterations");
}

//Synapse* SimEnv::connect(SimElemCommon* pre, SimElemCommon* post) {
//    Synapse* c = new Synapse(pre, post);
//    connections.push_back(c);
//    return c;   
//}
