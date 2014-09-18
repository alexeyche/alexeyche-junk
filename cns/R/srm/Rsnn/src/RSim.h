#ifndef RSIM_H
#define RSIM_H

#include <Rcpp.h>

#include "RConstants.h"
#include "conv_util.h"

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
    void printLayers() {
        for(size_t li=0; li < s->layers->size; li++) {
            s->layers->array[li]->printLayer(s->layers->array[li]);
        }
    }
    void print() {
        std::cout << "RSim object with " << s->layers->size << " layer(s):" << "\n";
        for(size_t li=0; li < s->layers->size; li++) {
            std::cout <<"\tlayer " << li+1 << ". size: " << s->layers->array[li]->N << "\n";
        }
    }
    void setInputSpikes(Rcpp::List sp) {
        assert(s->c->M == sp.size());
        simSetInputSpikes(s, RListToSpikesList(sp));
    }    
    Sim *s;
};



#endif
