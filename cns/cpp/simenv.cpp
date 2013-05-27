
#include "simenv.h"


Neurons* SimEnv::addNeuronGroup(int n) { 
    Neurons *elems = new Neurons(n);
    elements.push_back((SimBasic*)elems);
    return elems;
}

VoltMeter* SimEnv::addVoltMeter(Neurons *el) { 
    VoltMeter *elems = new VoltMeter(el);
    elements.push_back((SimBasic*)elems);
    return elems;
}

Poisson* SimEnv::addPoissonGroup(int n, double mHerz) { 
    Poisson *elems = new Poisson(mHerz);
    elements.push_back((SimBasic*)elems);
    return elems;
}


void SimEnv::runSimulation(double time_ms) {
    for(double t; t < time_ms; t+=tau) {
        for(size_t i=0; i<elements.size(); i++) {
            elements[i]->computeMe(tau);        
        }
        for(size_t i=0; i<connections.size(); i++) {
            connections[i]->computeMe(tau);        
        }
    }
}

Connection* SimEnv::connect(SimElemCommon* pre, SimElemCommon* post) {
	Connection* c = new Connection(pre, post);
	connections.push_back(c);
    return c;	
}
