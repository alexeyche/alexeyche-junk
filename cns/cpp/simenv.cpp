
#include "simenv.h"

void SimEnv::addNeuronGroup(int n) { 
    SimElem *neurons = new Neurons(n);
    elements.push_back(neurons);
}

void SimEnv::runSimulation(double time_ms) {
    for(double t; t < time_ms; t+=tau) {
        for(size_t i=0; i<elements.size(); i++) {
            elements[i]->computeMe(tau);        
        }
    }
}

