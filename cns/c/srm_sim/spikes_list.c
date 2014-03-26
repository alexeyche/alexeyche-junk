
#include "spikes_list.h"

SpikesList* createSpikesList(size_t size_) {
    SpikesList* sl = (SpikesList*) malloc( sizeof(SpikesList) );
    sl->size = size_;
    sl->list = (doubleVector**) malloc( sl->size * sizeof(doubleVector*));
    for(size_t li=0; li<sl->size; li++) {
        sl->list[li] = TEMPLATE(createVector,double)();
    }
    return(sl);
}

void readSpikesFromMatrix(SpikesList *sl, Matrix *m) {
    assert(m->nrow < sl->size);
    for(size_t i=0; i<m->nrow; i++) {
        for(size_t j=0; j<m->ncol; j++) {
            double el = getMatrixElement(m, i, j);
            if ( (el == 0) && (! ((m->ncol>j) && (getMatrixElement(m,i,j+1) > 0)) ) ) break;
            TEMPLATE(insertVector,double)(sl->list[i], el);
        }
    }
}

void deleteSpikesList(SpikesList *sl) {
    for(size_t li=0; li<sl->size; li++) {
        TEMPLATE(deleteVector,double)(sl->list[li]);
    }
    free(sl->list);
    sl->size = 0;
    free(sl);
}

void printSpikesList(SpikesList *sl) {
    printf("SpikeList with %d size:\n", sl->size);
    for(size_t si=0; si<sl->size; si++) {
        printf("%zu: ", si);
        for(size_t sp_i=0; sp_i < sl->list[si]->size; sp_i++) {
            printf("%f, ", sl->list[si]->array[sp_i]);
        }
        printf("\n");
    }
}
