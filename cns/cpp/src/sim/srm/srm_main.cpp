


#include <sim/core.h>
#include <sim/socket/sim_socket_core.h>

#include "sim.h"
#include "connections.h"
#include "research.h"

#include "entropy.h"

using namespace srm;

PROGRAM_INFO("SRM SIM", "Spike Resoponce Model simulator"); 
PARAM_INT("seed", "seed", "s", time(NULL));


int main(int argc, char** argv)
{
    CLI::ParseCommandLine(argc, argv);
	const int seed = CLI::GetParam<int>("seed");   
    std::srand(seed);
    Sim s;
    SrmNeuron* n = new SrmNeuron();
    double w_start = 3;
    n->add_input(new DetermenisticNeuron("3  10 11 12"), w_start);
    n->add_input(new DetermenisticNeuron("4  13 14 15"), w_start);
    n->add_input(new DetermenisticNeuron("5  15 16 17"), w_start);
    n->add_input(new DetermenisticNeuron("6 "), w_start);
    n->add_input(new DetermenisticNeuron("7 "), w_start);
    n->add_input(new DetermenisticNeuron("8 "), w_start);
//    TimeSeriesGroup g(100, 0*ms, 100); 
//    g.loadPatternFromFile("/var/tmp/d1.csv", 100*ms, 100);
//    send_arma_mat(g.patterns[0].pattern, "pattern");
//    send_arma_mat(g.patterns[0].pattern, "d1_stat");
//    g.loadPatternFromFile("/var/tmp/d2.csv", 100*ms, 0.5);
//    send_arma_mat(g.patterns[1].pattern, "d2_stat");
//    srm::connectFeedForward(&g, n, 0.2);
//    s.addNeuronGroup(&g);
    s.addNeuron(n);
    s.addStatListener(n, TStatListener::Spike);
    s.addStatListener(n, TStatListener::Prob);
    s.run(100*ms);

    EntropyCalc ec(n, 0, 100);
    Timer::Start("perf");
    ec.IntPerfomance();
    Timer::Stop("perf");
    
//    vec pp(100);
//    for(size_t num=0; num<100; num++) {        
//        double t_run = 0.05*sec;
//        s.run(t_run);
//        pp(num) = survFunction(t_run, n);
//        n->y.clean();
//    }        
//    send_arma_mat(pp, "pp");
//    for(size_t ni=0; ni<s.stoch_elem.size(); ni++) {
//        Log::Info << "id: " << s.stoch_elem[ni]->id() << "\n";
//        s.stoch_elem[ni]->y.print();
//    }
}

