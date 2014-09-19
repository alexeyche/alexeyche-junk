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
        configureNetSpikesSim(s, rc->c);
        configureSynapses(s, rc->c);
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
    void printConn() {
        printConnMap(s->ns);
    }
    void printInputSpikes() {
        printInputSpikesQueue(s->ns);
    }
    void setInputSpikes(Rcpp::List sp) {
        if(s->ctx->c->M != sp.size()) {
            std::ostringstream ss; ss << "Need list with " << s->ctx->c->M << " elements for each input" << "\n";
            Rcpp::stop(ss.str());
        }
        SpikesList *sl = RListToSpikesList(sp);
        simSetInputSpikes(s, sl);
        deleteSpikesList(sl); 
    }    

    Rcpp::List run() {
       runSim(s); 
       return SpikesListToRList(s->ns->net);
    }
    
    Rcpp::List getLayerStat(size_t layer_id) {
        if(layer_id >= s->layers->size) {
            std::ostringstream ss; ss << "Can't find layer number " << layer_id << "\n";
            Rcpp::stop(ss.str());
        }
        
        pMatrixVector *mv = TEMPLATE(createVector,pMatrix)();
        s->layers->array[layer_id]->saveStat(s->layers->array[layer_id], mv);
        
        Rcpp::List l = MatrixVectorToRList(mv);
        TEMPLATE(deleteVector,pMatrix)(mv);
        return l;
    }
    Rcpp::List W() {
        Rcpp::List l(s->layers->size);
        for(size_t li=0; li<s->layers->size; li++) {
            l[li] = DoublesAllocToRMatrix(s->layers->array[li]->W, s->layers->array[li]->nconn, s->layers->array[li]->N);
        }
        return l;
    }
private:
    Sim *s;
};



#endif
