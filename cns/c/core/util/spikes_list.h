#ifndef SPIKES_LIST_H
#define SPIKES_LIST_H

#include <core/util/matrix.h>
#include <core/util/util_vector.h>
#include <core/util/io.h>

// SpikesList :

typedef struct {
    doubleVector **list;
    size_t size;
} SpikesList;

typedef struct {
    SpikesList *sl;
    doubleVector *timeline;
    doubleVector *pattern_classes;
} SpikePatternsList;

SpikesList* createSpikesList(size_t size_);
void readSpikesFromMatrix(SpikesList *sl, Matrix *m);
void deleteSpikesList(SpikesList *sl);
void printSpikesList(SpikesList *sl);
SpikesList* spikesMatrixToSpikesList(Matrix *m);
Matrix* spikesListToSpikesMatrix(SpikesList *sl);

SpikePatternsList* createSpikePatternsList(size_t n);
SpikePatternsList* readSpikePatternsListFromFile(const char *filename);
void saveSpikePatternsListToFile(SpikePatternsList *spl, const char *filename);

#endif
