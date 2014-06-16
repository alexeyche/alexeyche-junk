
#include <core.h>

#include <layer/adex.h>
#include <args/prep_opts.h>

#include <util/spikes_list.h>
#include <util/io.h>
#include <util/util.h>

#include <prep/prepare_ts.h>

int main(int argc, char **argv) {
    ArgOptionsPrep a = parsePrepOptions(argc, argv);
    Constants *c = createConstants(a.const_filename);

    
    bool saveStat = false;
    if(a.stat_file) {
        saveStat = true;
    }

    pMatrixVector* ts_data = readMatrixList(a.input_file);
    pMatrixVector* ts_labels = readMatrixList(a.input_labels_file);
    assert(ts_labels->size == 1);
    assert(ts_labels->array[0]->nrow == ts_data->size);
    
    pMatrixVector *out_data = TEMPLATE(createVector,pMatrix)();
    assert(ts_data->size > 0);

    pMatrixVector *ts_data_pr = processTimeSeriesSet(ts_data, c);    
    indVector *ts_indices = TEMPLATE(createVector,ind)();
    for(size_t ti=0; ti< ts_data_pr->size; ti++) {
        TEMPLATE(insertVector,ind)(ts_indices, ti);    
    }
    srand(time(NULL));
    shuffleIndVector(ts_indices);
    
    SpikesList *net = createSpikesList(c->M);
    doubleVector *ts_labels_current = TEMPLATE(createVector,double)();
    double t = 0;
    doubleVector *timeline = TEMPLATE(createVector,double)();
    AdExLayer *l = createAdExLayer(c->M, saveStat);
    for(size_t ts_i=0; ts_i < ts_indices->size; ts_i++) {
        Matrix *ts = ts_data_pr->array[ ts_indices->array[ts_i] ];
        size_t nsamples = ts->ncol;
        toStartValuesAdExLayer(l, c);
        size_t j;
        for(j = 0; j < nsamples; t+= c->preproc->dt, j++) {
            for(size_t ni=0; ni < l->N; ni++) {
                double I = getMatrixElement(ts, ni, j);
                propagateCurrentAdExLayer(l, &ni, &I);
                simulateAdExLayerNeuron(l, &ni, c);
                if(l->fired[ni] == 1) {
                    TEMPLATE(insertVector,double)(net->list[ni], c->preproc->mult*t);
                    l->fired[ni] = 0;
                }
            }
        }
        t += 250/c->preproc->mult;
        TEMPLATE(insertVector,double)(timeline, c->preproc->mult*t);
        TEMPLATE(insertVector,double)(ts_labels_current, getMatrixElement(ts_labels->array[0], ts_indices->array[ts_i], 0) );
    }
    Matrix *spikes = vectorArrayToMatrix(net->list, net->size);
    TEMPLATE(insertVector,pMatrix)(out_data, spikes);    
    
    Matrix *timeline_m = vectorArrayToMatrix(&timeline, 1);
    transposeMatrix(timeline_m);
    TEMPLATE(insertVector,pMatrix)(out_data, timeline_m);    
    
    Matrix *classes = vectorArrayToMatrix(&ts_labels_current, 1);
    transposeMatrix(classes);
    TEMPLATE(insertVector,pMatrix)(out_data, classes);    

    saveMatrixList(a.output_file, out_data);

    if(l->saveStat) {
        pMatrixVector *stat_data = TEMPLATE(createVector,pMatrix)();
        for(size_t i=0; i<ts_data_pr->size; i++) {
            TEMPLATE(insertVector,pMatrix)(stat_data, copyMatrix(ts_data_pr->array[i]));
        }        
        Matrix *Vs = vectorArrayToMatrix(l->stat_V, l->N);
        TEMPLATE(insertVector,pMatrix)(stat_data, Vs);
        Matrix *ws = vectorArrayToMatrix(l->stat_w, l->N);
        TEMPLATE(insertVector,pMatrix)(stat_data, ws);


        saveMatrixList(a.stat_file, stat_data);

        TEMPLATE(deleteVector,pMatrix)(stat_data);
    }

    TEMPLATE(deleteVector,double)(timeline);
    deleteAdExLayer(l);
    deleteSpikesList(net);
    deleteConstants(c);
    TEMPLATE(deleteVector,pMatrix)(out_data);
    TEMPLATE(deleteVector,pMatrix)(ts_data_pr);
    TEMPLATE(deleteVector,pMatrix)(ts_data);
    return(0);
}
