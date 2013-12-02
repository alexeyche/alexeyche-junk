


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
    connect(&g, &g, TConnType::AllToAll, 0.1);
    for(size_t ni=0; ni<g.size(); ni++) {
        Log::Info << "neuron " << g.group[ni]->id() << " is connected to : ";    
        for(size_t wi=0; wi<g.group[ni]->w.size(); wi++) {
            Log::Info << g[ni]->in[wi]->id() << ";w=" << g.group[ni]->w[wi] << ", ";
        }
        Log::Info << "\n";
    }

//    TimeSeriesGroup g(100, 0*ms, 100); 
//    g.loadPatternFromFile("/var/tmp/d1.csv", 500*ms, 100);
//    send_arma_mat(g.patterns[0].pattern, "d1_stat");
//    g.loadPatternFromFile("/var/tmp/d2.csv", 100*ms, 0.5);
//    send_arma_mat(g.patterns[1].pattern, "d2_stat");
//    srm::connectFeedForward(&g, &n, 1);
    
    if(mode == "run") {    
        s.run(100*ms, 0.5);
    } else 
    if(mode == "learn") {
        //Log::Info << "weights before:\n";
        //for(size_t wi=0; wi<n.w.size(); wi++) { Log::Info << n.w[wi] << ", "; } Log::Info << "\n";

        //TEntropyGrad eg(&n, 0, 40);
        //TEntropyCalc ec(&n, 0, 40);
        //for(size_t i=0; i<10; i++) {
        //    vec dHdw = eg.grad();
        //    for(size_t wi=0; wi<n.w.size(); wi++) { n.w[wi] -= learning_rate * dHdw(wi); }
        //    Log::Info << "Epoch(" << i << "): ";
        //    Log::Info << "Grad: ";
        //    for(size_t wi=0; wi<n.w.size(); wi++) { Log::Info << dHdw(wi) << ", "; }
        //    if(i % 10 == 0) {
        //        Log::Info << " H: ";
        //        double H = ec.run(2);
        //        Log::Info << H;
        //    }            
        //    Log::Info << "\n";
        //}        
        //s.run(100*ms, 0.5);
        //Log::Info << "weights after:\n";
        //for(size_t wi=0; wi<n.w.size(); wi++) { Log::Info << n.w[wi] << ", "; } Log::Info << "\n";
    }        
}

