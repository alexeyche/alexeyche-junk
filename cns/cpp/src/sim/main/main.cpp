
#include "simenv.h"

#include <sim/core.h>
#include <sim/socket/sim_socket_core.cpp>

using namespace sim;

PROGRAM_INFO("SIM", "spiking neural network simulation framework"); 
PARAM_DOUBLE("neurons", "scale of network", "n", 10);

int main(int argc, char** argv) {	
    CLI::ParseCommandLine(argc, argv);
	double n = CLI::GetParam<double>("neurons");

    SimEnv env;

    Poisson *p1 = env.addPoissonElem(10, 50, 10); // 10 mHerz - freq, 50 ms - long, 10 mA - out value
	
    NeuronOptions n_exc(80);
    n_exc.axonOpts.setGenerator(new UnifRandGen(1,20));
    n_exc.a = 0.02;
    n_exc.d = 8;
    NeuronOptions n_inh(20);    
    n_inh.axonOpts.setGenerator(new SampleRandGen("1","1"));    
    n_inh.a = 0.1;
    n_inh.d = 2;
    NeuronGroupOptions n_opts;    
    n_opts.add(&n_exc);
    n_opts.add(&n_inh);    
    Neurons *n1 = env.addNeuronGroup(n_opts);    


    Connection *c = env.connect(p1,n1);    
    
    // VoltMeter *v = env.addVoltMeter(n1);
    // env.runSimulation(SimOptions(0.25, 100)); // tau(ms), simulation time (ms)
    
    // send_arma_mat(v->acc, "V");    
}
