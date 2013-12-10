


#include <sim/core.h>
#include <sim/data/save.h>
#include <sim/data/load.h>

#include "cfg.h"
#include "sim.h"
#include "connections.h"
#include "research.h"


using namespace srm;

PROGRAM_INFO("SRM SIM", "Spike Responce Model simulator"); 
PARAM_INT("seed", "seed", "s", time(NULL));
PARAM_DOUBLE("rate", "learning rate", "r", 1);
PARAM_DOUBLE("simtime", "time to simulate(ms)", "t", 1100);
PARAM_STRING("mode", "modes for srm sim: \'run\', \'learn\', \'stdp\'", "m", "run");
PARAM_INT("epoch", "Num of epochs in learning mode", "e", 100);
PARAM_STRING("pattern", "specific pattern", "p", "");
PARAM_DOUBLE("pattdur", "specific pattern duration", "d", 500);

bool fileExist( const std::string& name )
{
     std::ifstream f(name.c_str());  // New enough C++ library will accept just name
     return f.is_open();
}

int main(int argc, char** argv)
{
    CLI::ParseCommandLine(argc, argv);
	const int seed = CLI::GetParam<int>("seed");   
    const std::string mode = CLI::GetParam<std::string>("mode");
    const std::string specpattern = CLI::GetParam<std::string>("pattern");
    const double specpattdur = CLI::GetParam<double>("pattdur");

    const double learning_rate = CLI::GetParam<double>("rate");
    const double simtime = CLI::GetParam<double>("simtime");
    const int epoch = CLI::GetParam<int>("epoch");
    
    std::srand(seed);
    
    Sim s(learning_rate);
    SrmNeuronGroup g(1);
    mat start_w(g.size(),g.size(), fill::randn);
//    start_w = 1.7 + start_w*0.2;
    start_w.fill(5);
    connect(&g, &g, TConnType::AllToAll, start_w);


    TimeSeriesGroup tsg(10, 10*ms, 100); 
    if(specpattern == "") {
        tsg.loadPatternFromFile("/var/tmp/dtest.csv", 10*ms, 1);
//        tsg.loadPatternFromFile("/var/tmp/d1.csv", 500*ms, 1);
//        tsg.loadPatternFromFile("/var/tmp/d2.csv", 500*ms, 1);
//        tsg.loadPatternFromFile("/var/tmp/d3.csv", 500*ms, 1);
    } else {
        tsg.loadPatternFromFile(specpattern, specpattdur*ms, 1);
    }

    mat start_w_ff(tsg.size(),g.size(), fill::randn);
    start_w_ff.fill(5);
    connect(&tsg, &g, TConnType::FeedForward, start_w_ff);
                
//    for(size_t ni=0; ni<g.size(); ni++) {
//        Log::Info << "neuron " << g.group[ni]->id() << " is connected to : ";    
//        for(size_t wi=0; wi<g.group[ni]->w.size(); wi++) {
//            Log::Info << g[ni]->in[wi]->id() << ";w=" << g.group[ni]->w[wi] << ", ";
//        }
//        Log::Info << "\n";
//    }   
//    return 0;
    s.addNeuronGroup(&tsg);
    s.addNeuronGroup(&g);
    s.addStatListener(&g);
    TRunType::ERunType rt;   
    if(mode == "run") {    
        rt = TRunType::Run;
        mat saved_weights(g.size(), g.group[0]->w.size());
        if (fileExist("/var/tmp/weights.csv"))  {
            data::Load("/var/tmp/weights.csv", saved_weights);
            for(size_t ni=0; ni<g.group.size(); ni++) {
                SrmNeuron *n =  dynamic_cast<SrmNeuron*>(g.group[ni]);
                if(n) {
                    Log::Info << "neuron " << ni << "\n";
                    for(size_t wi=0; wi<n->w.size(); wi++) { Log::Info << saved_weights(ni,wi) << ", "; n->w[wi] = saved_weights(ni,wi); }
                    Log::Info << "\n";
                }            
            }
        }            
    } 
    if(mode == "stdp") {    
        rt = TRunType::RunAndLearnSTDP;
    } 
    if(mode == "learn") {    
        rt = TRunType::RunAndLearnLogLikelyhood;
    }        
    vec weights(g.group.size(), g.group[0]->w.size(), fill::zeros);
    for(size_t ep=0; ep<epoch; ep++) {
        for(size_t ni=0; ni<g.group.size(); ni++) { g.group[ni]->y.clean(); }
        bool send=false;
        if((ep == epoch-1)||(ep == 0)) {
            send=true;
        }
        s.run(simtime*ms, 0.5, rt, true, send);
        Log::Info << "weight after:\n";
        for(size_t ni=0; ni<g.group.size(); ni++) {
            SrmNeuron *n =  dynamic_cast<SrmNeuron*>(g.group[ni]);
            if(n) {
                Log::Info << "neuron " << ni << "\n";
                for(size_t wi=0; wi<n->w.size(); wi++) { Log::Info << n->w[wi] << ", "; weights(ni,wi) = n->w[wi]; }
                Log::Info << "\n";
            }            
        }        
    }        
    if((rt == TRunType::RunAndLearnSTDP)||(rt == TRunType::RunAndLearnLogLikelyhood)) {
        data::Save("/var/tmp/weights.csv", weights);
    }        
    send_arma_mat(weights, "weights");
}

