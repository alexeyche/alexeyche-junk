


#include <sim/core.h>
#include <sim/socket/sim_socket_core.h>

#include "cfg.h"
#include "sim.h"
#include "connections.h"
#include "research.h"

#include "entropy_grad.h"

using namespace srm;

PROGRAM_INFO("SRM SIM", "Spike Responce Model simulator"); 
PARAM_INT("seed", "seed", "s", time(NULL));
PARAM_DOUBLE("rate", "learning rate", "r", 1);
PARAM_STRING("mode", "modes for srm sim: \'run\', \'learn\'", "m", "run");
PARAM_INT("epoch", "Num of epochs in learning mode", "e", 100);

int main(int argc, char** argv)
{
    CLI::ParseCommandLine(argc, argv);
	const int seed = CLI::GetParam<int>("seed");   
    const std::string mode = CLI::GetParam<std::string>("mode");
    const double learning_rate = CLI::GetParam<double>("rate");
    const int epoch = CLI::GetParam<int>("epoch");
    
    std::srand(seed);
    
    Sim s;
    SrmNeuronGroup g(10);
    connect(&g, &g, TConnType::AllToAll, 3);


    TimeSeriesGroup tsg(50, 0*ms, 100); 
    tsg.loadPatternFromFile("/var/tmp/d1.csv", 2000*ms, 100);
//    send_arma_mat(tsg.patterns[0].pattern, "d1_stat");
//    return 0;
//    g.loadPatternFromFile("/var/tmp/d2.csv", 100*ms, 0.5);
//    send_arma_mat(g.patterns[1].pattern, "d2_stat");
    connect(&tsg, &g, TConnType::FeedForward, 3);
    
//    for(size_t ni=0; ni<g.size(); ni++) {
//        Log::Info << "neuron " << g.group[ni]->id() << " is connected to : ";    
//        for(size_t wi=0; wi<g.group[ni]->w.size(); wi++) {
//            Log::Info << g[ni]->in[wi]->id() << ";w=" << g.group[ni]->w[wi] << ", ";
//        }
//        Log::Info << "\n";
//    }   
    s.addNeuronGroup(&tsg);
    s.addNeuronGroup(&g);
    s.addStatListener(&g);
    TRunType::ERunType rt;   
    if(mode == "run") {    
        rt = TRunType::Run;
    } 
    if(mode == "learn") {    
        rt = TRunType::RunAndLearn;
    } 
    
    s.run(300*ms, 0.5, rt);
    Log::Info << "weight after:\n";
    for(size_t ni=0; ni<g.group.size(); ni++) {
        SrmNeuron *n =  dynamic_cast<SrmNeuron*>(g.group[ni]);
        if(n) {
            Log::Info << "neuron " << ni << "\n";
            for(size_t wi=0; wi<n->w.size(); wi++) { Log::Info << n->w[wi] << ", "; }
            Log::Info << "\n";
        }            
    }        
}

