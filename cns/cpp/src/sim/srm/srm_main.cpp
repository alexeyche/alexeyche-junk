


#include <sim/core.h>
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
    double w_start = 5;
//    n->add_input(new PoissonNeuron(100/sec), w_start);
//    n->add_input(new PoissonNeuron(100/sec), w_start);
//    n->add_input(new PoissonNeuron(100/sec), w_start);
//    n->add_input(new PoissonNeuron(10/sec), w_start);
//    n->add_input(new PoissonNeuron(10/sec), w_start);
//    n->add_input(new PoissonNeuron(10/sec), w_start);

    n->add_input(new DetermenisticNeuron("5 10 31 41"), w_start);
    n->add_input(new DetermenisticNeuron("6 11 32 42"), w_start);
    n->add_input(new DetermenisticNeuron("7 12 33 43"), w_start);
    n->add_input(new DetermenisticNeuron("8 13 34 44"), w_start);
    n->add_input(new DetermenisticNeuron("9 14 35 45"), w_start);


    s.addRecNeuron(n);
    s.run(0.1*sec);
    for(size_t ni=0; ni<s.stoch_elem.size(); ni++) {
        Log::Info << "id: " << s.stoch_elem[ni]->id() << "\n";
        s.stoch_elem[ni]->y.print();
    }
}

