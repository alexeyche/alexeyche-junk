#ifndef SPIKES_LIST_H
#define SPIKES_LIST_H

#include <util/matrix.h>
#include <util/util_vector.h>

// SpikesList :

typedef struct {
    doubleVector **list;
    size_t size;
} SpikesList;


SpikesList* createSpikesList(size_t size_);
void readSpikesFromMatrix(SpikesList *sl, Matrix *m);
void deleteSpikesList(SpikesList *sl);
void printSpikesList(SpikesList *sl);
SpikesList* spikesMatrixToSpikesList(Matrix *m);


#endif
