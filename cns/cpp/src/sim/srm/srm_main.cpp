


#include <sim/core.h>
#include <sim/socket/sim_socket_core.h>

#include "sim.h"

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
    double w_start = 10;
//    n->add_input(new PoissonNeuron(100/sec), w_start);
//    n->add_input(new PoissonNeuron(100/sec), w_start);
//    n->add_input(new PoissonNeuron(100/sec), w_start);
//    n->add_input(new PoissonNeuron(10/sec), w_start);
//    n->add_input(new PoissonNeuron(10/sec), w_start);
//    n->add_input(new PoissonNeuron(10/sec), w_start);

    TimeSeriesGroup g(10, 50*ms, 0.1); 
    g.loadPatternFromFile("/var/tmp/d1.csv", 100*ms, 0.5);
//    send_arma_mat(g.patterns[0].pattern, "d1_stat");
    g.loadPatternFromFile("/var/tmp/d2.csv", 100*ms, 0.5);
//    send_arma_mat(g.patterns[1].pattern, "d2_stat");
    s.addNeuronGroup(&g);

    s.addRecNeuron(n);
    s.addStatListener(n, TStatListener::Spike);
    s.addStatListener(n, TStatListener::Prob);

    s.run(5*sec);

    for(unsigned int gi=0; gi<g.group.size(); gi++) {
        send_arma_mat(g.group[gi].y, "ystat",&gi);
    }        

//    for(size_t ni=0; ni<s.stoch_elem.size(); ni++) {
//        Log::Info << "id: " << s.stoch_elem[ni]->id() << "\n";
//        s.stoch_elem[ni]->y.print();
//    }
}

