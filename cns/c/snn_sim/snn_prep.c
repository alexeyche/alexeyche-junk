
#include <core.h>

#include "adex_layer.h"
#include "arg_opt_prep.h"

#include <util/spikes_list.h>
#include <io.h>

int main(int argc, char **argv) {
    ArgOptionsPrep a = parsePrepOptions(argc, argv);
    Constants *c = createConstants(a.const_filename);
<<<<<<< HEAD
=======

    pMatrixVector* ts_data = readMatrixList(a.input_file);
    Matrix *ts = ts_data->array[0];
    size_t N = ts->nrow;
    size_t nsamples = ts->ncol;
>>>>>>> 318e8ea01391cb34992d34a456f378d5952f3290
    bool saveStat = false;
    if(a.stat_file) {
        saveStat = true;
    }
<<<<<<< HEAD

    pMatrixVector* ts_data = readMatrixList(a.input_file);
    pMatrixVector *out_data = TEMPLATE(createVector,pMatrix)();
    assert(ts_data->size > 0);
    size_t N = ts_data->array[0]->nrow;
    size_t nsamples = ts_data->array[0]->ncol;

    SpikesList *net = createSpikesList(N);
    double t = 0;
    doubleVector *timeline = TEMPLATE(createVector,double)();
    AdExLayer *l = createAdExLayer(N, saveStat);
    for(size_t ts_i=0; ts_i < ts_data->size; ts_i++) {
        Matrix *ts = ts_data->array[ts_i];
        toStartValuesAdExLayer(l, c);
        size_t j;
        for(j = 0; t < (ts_i+1)*nsamples*c->dt; t+= c->dt, j++) {
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
    TEMPLATE(insertVector,pMatrix)(out_data, timeline_m);    
=======
    AdExLayer *l = createAdExLayer(N, saveStat);
    SpikesList *net = createSpikesList(N);
    size_t j;
    double t;
    for(t=0, j = 0; t < nsamples*c->dt; t+= c->dt, j++) {
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
    pMatrixVector *out_data = TEMPLATE(createVector,pMatrix)();
    Matrix *spikes = vectorArrayToMatrix(net->list, net->size);
    TEMPLATE(insertVector,pMatrix)(out_data, spikes);
//    TEMPLATE(insertVector,pMatrix)(out_data, copyMatrix(ts_data->array[1]));
//    TEMPLATE(insertVector,pMatrix)(out_data, copyMatrix(ts_data->array[2]));

>>>>>>> 318e8ea01391cb34992d34a456f378d5952f3290
    saveMatrixList(a.output_file, out_data);

    if(l->saveStat) {
        pMatrixVector *stat_data = TEMPLATE(createVector,pMatrix)();
        Matrix *Vs = vectorArrayToMatrix(l->stat_V, l->N);
        TEMPLATE(insertVector,pMatrix)(stat_data, Vs);
        Matrix *ws = vectorArrayToMatrix(l->stat_w, l->N);
        TEMPLATE(insertVector,pMatrix)(stat_data, ws);

        saveMatrixList(a.stat_file, stat_data);

        TEMPLATE(deleteVector,pMatrix)(stat_data);
    }

<<<<<<< HEAD
    TEMPLATE(deleteVector,double)(timeline);
=======
>>>>>>> 318e8ea01391cb34992d34a456f378d5952f3290
    deleteAdExLayer(l);
    deleteSpikesList(net);
    deleteConstants(c);
    TEMPLATE(deleteVector,pMatrix)(out_data);
    TEMPLATE(deleteVector,pMatrix)(ts_data);
    return(0);
}
