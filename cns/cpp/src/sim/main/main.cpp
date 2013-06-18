
#include "simenv.h"

#include <sim/core.h>
#include <sim/socket/sim_socket_core.cpp>

using namespace sim;

PROGRAM_INFO("SIM", "spiking neural network simulation framework"); 
PARAM_DOUBLE("neurons", "scale of network", "n", 1000);

int main(int argc, char** argv) {	
    CLI::ParseCommandLine(argc, argv);
	double n = CLI::GetParam<double>("neurons");

    SimEnv env;

    Poisson *p1 = env.addPoissonElem(10, 2, 10); // 10 mHerz - freq, 50 ms - long, 10 mA - out value
	
    double exc_ratio = 0.8;
    NeuronOptions n_exc(800,"excitatory");
    
    n_exc.axonOpts.setGenerator(new UnifRandGen<vec>(1,20));
    
    n_exc.a = 0.02;
    n_exc.d = 8;
    
    NeuronOptions n_inh(200, "inhibitory");    
    n_inh.axonOpts.setGenerator(new SampleRandGen<vec>("1","1"));    
    n_inh.a = 0.1;
    n_inh.d = 2;
    NeuronGroupOptions n_opts;    
    n_opts.add(&n_exc);
    n_opts.add(&n_inh);    

    Neurons *n1 = env.addNeuronGroup(n_opts);    

    Connection *c = env.connect<Connection>(p1, n1);    

    int synapse_num = 500;
    // synaptic connection between excitatory and all other neurons    
    
    SynapticOptions syn_exc_all0(n1->getIndSubgroup("excitatory"), n1->getIndAll(), 0.1, synapse_num/2, AMPA, 5);  
    SynapticOptions syn_exc_all1(n1->getIndSubgroup("excitatory"), n1->getIndAll(), 0.1, synapse_num/2, NMDA, 150);
    
    
    // synaptic connection between inhibitory and excitatory
    SynapticOptions syn_inh_exc0(n1->getIndSubgroup("inhibitory"), n1->getIndSubgroup("excitatory"), 0.1, synapse_num/2, GABA_A, 6);
    SynapticOptions syn_inh_exc1(n1->getIndSubgroup("inhibitory"), n1->getIndSubgroup("excitatory"), 0.1, synapse_num/2, GABA_B, 150);

    SynapticGroupOptions syn_opt;
    syn_opt.add(&syn_exc_all0);
    syn_opt.add(&syn_exc_all1);
    syn_opt.add(&syn_inh_exc0);
    syn_opt.add(&syn_inh_exc1);

    Synapse *syn = env.addSynapse(n1, n1, syn_opt);
    
    StatCollector *v_n = env.addStatCollector(n1->V);    
    StatCollector *v_isyn = env.addStatCollector(n1->Isyn);    
    StatCollector *axon_cur = env.addStatCollector(n1->axon->V_in_cur);
    StatCollector *axon_d_cur = env.addStatCollector(n1->axon->delays_cur);
    StatCollector *syn_out = env.addStatCollector(syn->out);
    env.runSimulation(SimOptions(0.25, 500)); // tau(ms), simulation time (ms)
    
    send_arma_mat(v_n->acc, "V");  
    send_arma_mat(v_isyn->acc, "Isyn");
    send_arma_mat(axon_cur->acc, "axon_cur");
    send_arma_mat(axon_d_cur->acc, "axon_d_cur");
    send_arma_mat(syn_out->acc, "syn_out");
    
}
