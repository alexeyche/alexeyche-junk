
#include "simenv.h"
#include "core.h"
//$(CC) $(CFLAGS) -c -o $@
#include "socket/sim_socket_core.cpp"

int main(int argc, char** argv) {

    SimEnv env;

    Poisson *p1 = env.addPoissonGroup(1, 10);
    //std::cout << (SimElemCommon*)p1->size.n_out << " - p1 n_out" << std::endl;    
    Neurons *n1 = env.addNeuronGroup(10);
    //std::cout << (SimElemCommon*)n1->size.n_in << " - n1 n_in" << std::endl;
    
    Connection *c = env.connect(p1,n1);    
    
    VoltMeter *v = env.addVoltMeter(n1);
    env.runSimulation(SimOptions(0.25, 100)); // tau(ms), simulation time (ms)
    
    send_arma_mat(v->acc, "V");    
}
