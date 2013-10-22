

#include <sim/core.h>
#include "srm.h"

void epsp_test() {
    double Tmax = 100;
    double dt = 0.1;
    uvec t = linspace<uvec>(0.0, Tmax, (int)Tmax/dt);
    
    double fi = 10.0;
    double fj = 5.0; 
    for(size_t ti=0; ti<t.n_elem; ti++) {
        std::cout << srm::epsp(t[ti], fj, fi) << ",";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {
    CLI::ParseCommandLine(argc, argv);
    std::string test_name("all");
	if(argc>1) {
        test_name = std::string(argv[1]);
        Log::Info << "Tests name was found: " << test_name << "\n";
    }   
    Log::Info << "Tests started" << std::endl;
    if((test_name == "all") || (test_name == "epsp")) {
        Log::Info << "epsp test: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        epsp_test();
        Log::Info << "===============================================================" << std::endl;
    }        
    return 0;
}

