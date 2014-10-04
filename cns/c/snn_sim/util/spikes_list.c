
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
    assert(m->nrow <= sl->size);
    for(size_t i=0; i<m->nrow; i++) {
        for(size_t j=0; j<m->ncol; j++) {
            double el = getMatrixElement(m, i, j);
            if ( (el == 0) && ((m->ncol == j+1) || ((m->ncol>(j+1)) && (getMatrixElement(m,i,j+1) == 0)) ) ) break;
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
    printf("SpikeList with %zu size:\n", sl->size);
    for(size_t si=0; si<sl->size; si++) {
        printf("%zu: ", si);
        for(size_t sp_i=0; sp_i < sl->list[si]->size; sp_i++) {
            printf("%f, ", sl->list[si]->array[sp_i]);
        }
        printf("\n");
    }
}


SpikesList* spikesMatrixToSpikesList(Matrix *m) {
    SpikesList *sl = createSpikesList(m->nrow);
    readSpikesFromMatrix(sl, m);    
    return(sl);
}

SpikePatternsList* createSpikePatternsList(size_t n) {
    SpikePatternsList* spl = (SpikePatternsList*) malloc( sizeof(SpikePatternsList) );
    spl->sl = createSpikesList(n);
    spl->timeline = TEMPLATE(createVector,double)();
    spl->pattern_classes = TEMPLATE(createVector,double)();
    return(spl);
}

SpikePatternsList* readSpikePatternsListFromFile(const char *filename) {
    SpikePatternsList* spl = (SpikePatternsList*) malloc( sizeof(SpikePatternsList) );

    FileStream *fs = createInputFileStream(filename);
    pMatrixVector *ml = readMatrixList(fs, 3);
    Matrix *inp_m = ml->array[0];
    spl->sl = spikesMatrixToSpikesList(inp_m);

    spl->timeline = TEMPLATE(createVector,double)();
    spl->pattern_classes = TEMPLATE(createVector,double)();

    Matrix *timeline_m = ml->array[1];
    for(size_t ri=0; ri<timeline_m->nrow*timeline_m->ncol; ri++) {
        TEMPLATE(insertVector,double)(spl->timeline, timeline_m->vals[ri]);
    }
    Matrix *classes_m = ml->array[2];
    for(size_t ri=0; ri<classes_m->nrow*classes_m->ncol; ri++) {
        TEMPLATE(insertVector,double)(spl->pattern_classes, classes_m->vals[ri]);
    }
    return(spl);
}

void saveSpikePatternsListToFile(SpikePatternsList *spl, const char *filename) {
    pMatrixVector *ml = TEMPLATE(createVector,pMatrix)();
    TEMPLATE(insertVector,pMatrix)(
    FileStream *fs - createOuputFileStream(filename);

}

