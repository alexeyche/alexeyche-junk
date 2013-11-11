


#include <sim/core.h>
#include <sim/socket/sim_socket_core.h>

#include "sim.h"
#include "connections.h"
#include "research.h"

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
    TimeSeriesGroup g(100, 0*ms, 100); 
    g.loadPatternFromFile("/var/tmp/d1.csv", 100*ms, 100);
    send_arma_mat(g.patterns[0].pattern, "pattern");
//    send_arma_mat(g.patterns[0].pattern, "d1_stat");
//    g.loadPatternFromFile("/var/tmp/d2.csv", 100*ms, 0.5);
//    send_arma_mat(g.patterns[1].pattern, "d2_stat");
    srm::connectFeedForward(&g, n, 0.2);
    s.addNeuronGroup(&g);
    s.addNeuron(n);

    s.addStatListener(n, TStatListener::Spike);
    s.addStatListener(n, TStatListener::Prob);
    s.run(0.2*sec);
    Log::Info << "Sim finished\n";

    vec t = linspace<vec>(0, 0.2*sec, (int)(0.2*sec)/0.1);
    vec pp(t.n_elem, 2, fill::zeros);
    for(double ti=0; ti<t.n_elem; ti++) {
        pp(ti,0) = t(ti);
        pp(ti,1) = n->p(t(ti));
    }
    send_arma_mat(pp, "pp");
    survFunction(0.2*sec, n);
//    for(size_t ni=0; ni<s.stoch_elem.size(); ni++) {
//        Log::Info << "id: " << s.stoch_elem[ni]->id() << "\n";
//        s.stoch_elem[ni]->y.print();
//    }
}

