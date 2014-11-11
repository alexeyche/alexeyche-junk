

#include "run.h"



SpikePatternsList* runNeurons(pMatrixVector *ts_data, doubleVector *ts_labels, Constants *c, bool saveStat) {
    indVector *ts_indices = TEMPLATE(createVector,ind)();
    for(size_t ti=0; ti< ts_data->size; ti++) {
        TEMPLATE(insertVector,ind)(ts_indices, ti);    
    }
    srand(time(NULL));
    shuffleIndVector(ts_indices);

    double t = 0.0; 
    AdExLayer *l = createAdExLayer(c->preproc->N, saveStat);
    SpikePatternsList *spl = createSpikePatternsList(c->preproc->N);

    for(size_t ts_i=0; ts_i < ts_indices->size; ts_i++) {
        doubleVector *ts = doubleVectorFromMatrix(ts_data->array[ ts_indices->array[ts_i] ]);
        toStartValuesAdExLayer(l, c);
        size_t j;
        for(j = 0; j < ts->size; t+= c->preproc->dt, j++) {
            double I = ts->array[j];
            for(size_t ni=0; ni < l->N; ni++) {
                propagateCurrentAdExLayer(l, &ni, &I);
                simulateAdExLayerNeuron(l, &ni, c);
                if(l->fired[ni] == 1) {
                    TEMPLATE(insertVector,double)(spl->sl->list[ni], t);
                    l->fired[ni] = 0;
                }
            }
        }
        TEMPLATE(insertVector,double)(spl->timeline, t);
        TEMPLATE(insertVector,double)(spl->pattern_classes, ts_labels->array[ts_indices->array[ts_i] ]);
    }
    return(spl);
}


