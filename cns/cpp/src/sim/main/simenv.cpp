
#include "simenv.h"
#include "axon.h"

Neurons* SimEnv::addNeuronGroup(NeuronGroupOptions opts) { 
    Neurons *elems = new Neurons(opts);
    elements.push_back((SimBasic*)elems);
    return elems;
}

VoltMeter* SimEnv::addVoltMeter(Neurons *el) { 
    VoltMeter *elems = new VoltMeter(el);
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
    for(double t=0; t < so.time_ms; t+=so.tau_ms) {
        for(size_t i=0; i<elements.size(); i++) {
            elements[i]->computeMe(so.tau_ms);        
        }
        for(size_t i=0; i<connections.size(); i++) {
            connections[i]->computeMe(so.tau_ms);        
        }
    }
}

Connection* SimEnv::connect(SimElemCommon* pre, SimElemCommon* post) {
	Connection* c = new Connection(pre, post);
	connections.push_back(c);
    return c;	
}
