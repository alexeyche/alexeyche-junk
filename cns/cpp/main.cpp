
#include "simenv.h"

#include "credis.h"

int main(int argc, char** argv) {
    SimEnv env(0.25);

    Poisson *p1 = env.addPoissonGroup(1, 100);
    //std::cout << (SimElemCommon*)p1->size.n_out << " - p1 n_out" << std::endl;    
    Neurons *n1 = env.addNeuronGroup(100);
    //std::cout << (SimElemCommon*)n1->size.n_in << " - n1 n_in" << std::endl;
    
    Connection *c = env.connect(p1,n1);
    std::cout << "conn mat: " << c->connMat.n_rows << ":" <<  c->connMat.n_cols << std::endl;      
    VoltMeter *v = env.addVoltMeter(n1);
    env.runSimulation(100);  
    
    for(size_t i=0; i< v->acc.size(); i++) {
        v->acc[i].print();
    }
}
