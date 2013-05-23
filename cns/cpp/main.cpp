
#include "neurons.h"

int main(int argc, char** argv) {
    SimEnv env(0.25);
    env.addNeuronGroup(100);
    env.runSimulation(300);  
    return 0;
}
