
#include "simenv.h"

#include <sim/core.h>
#include <sim/socket/sim_socket_core.cpp>

int main(int argc, char** argv) {

    SimEnv env;

    Poisson *p1 = env.addPoissonElem(10, 50, 10); // 10 mHerz - freq, 50 ms - long, 10 mA - out value
    
    Neurons *n1 = env.addNeuronGroup(10);
    
    
    Connection *c = env.connect(p1,n1);    
    
    VoltMeter *v = env.addVoltMeter(n1);
    env.runSimulation(SimOptions(0.25, 100)); // tau(ms), simulation time (ms)
    
    send_arma_mat(v->acc, "V");    
}
