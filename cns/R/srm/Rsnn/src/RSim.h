#ifndef RSIM_H
#define RSIM_H

#include <Rcpp.h>

#include "RConstants.h"

extern "C" {
    #include <sim/sim.h>
}


class RSim {
public:
    RSim(RConstants *rc, unsigned char statLevel, size_t nthreads) {
        s = createSim(nthreads, statLevel, rc->c);
        configureLayersSim(s, rc->c);
    }
    ~RSim() {
        deleteSim(s);    
    }
    void print() {
        std::cout << "RSim object with " << s->layers->size << " layer(s):" << "\n";
        for(size_t li=0; li < s->layers->size; li++) {
            std::cout <<"\tlayer " << li+1 << ". size: " << s->layers->array[li]->N << "\n";
        }
    }
    Sim *s;
};



#endif
