
#include <core.h>

#include <layer/adex.h>
#include <args/prep_opts.h>

#include <util/spikes_list.h>
#include <util/io.h>

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

    SpikesList *net = createSpikesList(c->M);
    double t = 0;
    doubleVector *timeline = TEMPLATE(createVector,double)();
    AdExLayer *l = createAdExLayer(c->M, saveStat);
    for(size_t ts_i=0; ts_i < ts_data_pr->size; ts_i++) {
        Matrix *ts = ts_data_pr->array[ts_i];
        size_t nsamples = ts->ncol;
        toStartValuesAdExLayer(l, c);
        size_t j;
        for(j = 0; t < (ts_i+1)*nsamples*c->preproc->dt; t+= c->preproc->dt, j++) {
            for(size_t ni=0; ni < l->N; ni++) {
                double I = getMatrixElement(ts, ni, j);
                propagateCurrentAdExLayer(l, &ni, &I);
                simulateAdExLayerNeuron(l, &ni, c);
                if(l->fired[ni] == 1) {
                    TEMPLATE(insertVector,double)(net->list[ni], t);
                    l->fired[ni] = 0;
                }
            }
        }
        TEMPLATE(insertVector,double)(timeline, t);
    }
    Matrix *spikes = vectorArrayToMatrix(net->list, net->size);
    TEMPLATE(insertVector,pMatrix)(out_data, spikes);    
    Matrix *timeline_m = vectorArrayToMatrix(&timeline, 1);
    transposeMatrix(timeline_m);
    TEMPLATE(insertVector,pMatrix)(out_data, timeline_m);    
    Matrix *classes = copyMatrix(ts_labels->array[0]);
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
