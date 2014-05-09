
#include <core.h>

#include "adex_layer.h"
#include "arg_opt_prep.h"

#include <util/spikes_list.h>
#include <io.h>

int main(int argc, char **argv) {
    ArgOptionsPrep a = parsePrepOptions(argc, argv);
    Constants *c = createConstants(a.const_filename);

    pMatrixVector* ts_data = readMatrixList(a.input_file);
    Matrix *ts = ts_data->array[0];
    size_t N = ts->nrow;
    size_t nsamples = ts->ncol;
    bool saveStat = false;
    if(a.stat_file) {
        saveStat = true;
    }
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

    deleteAdExLayer(l);
    deleteSpikesList(net);
    deleteConstants(c);
    TEMPLATE(deleteVector,pMatrix)(out_data);
    TEMPLATE(deleteVector,pMatrix)(ts_data);
    return(0);
}
