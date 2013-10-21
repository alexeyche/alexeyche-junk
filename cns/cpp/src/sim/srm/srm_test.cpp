

#include <sim/core.h>
#include "srm.h"

void epsp_test() {
    double Tmax = 100;
    double dt = 0.1;
    uvec t = linspace<uvec>(0.0, Tmax, (int)Tmax/dt);
    
    double fi = 10.0;
    double fj = 5.0; 
    for(size_t ti=0; ti<t.n_elem; ti++) {
        std::cout << srm::epsp(t[ti], fj, fi) << " ";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {
    CLI::ParseCommandLine(argc, argv);
    epsp_test();
    return 0;
}

