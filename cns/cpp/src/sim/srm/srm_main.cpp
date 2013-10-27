


#include <sim/core.h>
#include "sim.h"

using namespace srm;

int main(int argc, char** argv)
{
    CLI::ParseCommandLine(argc, argv);
    Sim s;
    SrmNeuron* n = new SrmNeuron();
    double w_start = 0.5;
    n->add_input(new PoissonNeuron(100*sec), w_start);
    n->add_input(new PoissonNeuron(100*sec), w_start);
    n->add_input(new PoissonNeuron(100*sec), w_start);
    n->add_input(new PoissonNeuron(10*sec), w_start);
    n->add_input(new PoissonNeuron(10*sec), w_start);
    n->add_input(new PoissonNeuron(10*sec), w_start);

    s.addRecNeuron(n);
    s.run(100);
    for(size_t ni=0; ni<s.stoch_elem.size(); ni++) {
        Log::Info << "id: " << s.stoch_elem[ni]->id() << "\n";
        s.stoch_elem[ni]->y.print();
    }
}

