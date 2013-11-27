


#include <sim/core.h>
#include <sim/socket/sim_socket_core.h>

#include "cfg.h"
#include "sim.h"
#include "connections.h"
#include "research.h"

#include "entropy_grad.h"

using namespace srm;

PROGRAM_INFO("SRM SIM", "Spike Resoponce Model simulator"); 
PARAM_INT("seed", "seed", "s", time(NULL));


int main(int argc, char** argv)
{
    CLI::ParseCommandLine(argc, argv);
	const int seed = CLI::GetParam<int>("seed");   
    std::srand(seed);
    Sim s;
    SrmNeuron n;

    double w_start = 5;
//    n.add_input(new DetermenisticNeuron("3  10 11 12"), w_start);
//    n.add_input(new DetermenisticNeuron("4  13 14 15"), w_start);
//    n.add_input(new DetermenisticNeuron("5  15 16 17"), w_start);
//    n.add_input(new DetermenisticNeuron("6 "), w_start);
//    n.add_input(new DetermenisticNeuron("7 "), w_start);
//    n.add_input(new DetermenisticNeuron("8 "), w_start);
    
    TimeSeriesGroup g(100, 0*ms, 100); 
    g.loadPatternFromFile("/var/tmp/d1.csv", 500*ms, 100);
    send_arma_mat(g.patterns[0].pattern, "d1_stat");
//    g.loadPatternFromFile("/var/tmp/d2.csv", 100*ms, 0.5);
//    send_arma_mat(g.patterns[1].pattern, "d2_stat");
    srm::connectFeedForward(&g, &n, 1);
    
    s.addNeuronGroup(&g);
    s.addNeuron(&n);

//    s.addStatListener(n, TStatListener::Spike);
    s.addStatListener(&n, TStatListener::Pot);
    s.addStatListener(&n, TStatListener::Prob);
    s.run(100*ms, 0.5);

//    TEntropyGrad eg(&n);
//    eg.gradNoSpike();
    TEntropyGrad eg(n, 0, 100);
    vec dHdw = eg.grad();
    dHdw.print();
//    Log::Info << "grad 1 spike: " << gns << "\n";
//    double Hall =0 ;
//    for(double T=0; T<80; T+=20) {
//        TEntropyGrad eg(&n,T, T+20);    
//        double H = ec.run(2);
//        Hall += H;
//        Log::Info <<  "H = " << H << "\n"; 
//    }
}

