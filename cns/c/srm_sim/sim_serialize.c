

#include "sim.h"

#define MATRIX_PER_LAYER 4
void loadLayersFromFile(Sim *s, const char *model_fname, Constants *c, bool saveStat) {
    s->c = c;
    pMatrixVector* data = readMatrixList(model_fname);
    assert(data != NULL);
    size_t net_size = s->c->M;
    for(size_t li=0; li< s->c->layers_size->size; li++) {
        pMatrixVector* data_layer = TEMPLATE(createVector,pMatrix)();
        for(size_t di=0; di < MATRIX_PER_LAYER; di++) {
            TEMPLATE(insertVector,pMatrix)(data_layer, data->array[di + MATRIX_PER_LAYER*li]);
        }
        size_t neurons_idx = s->c->M;
        SRMLayer *l = createSRMLayer(s->c->layers_size->array[li], &neurons_idx, saveStat);
        loadSRMLayer(l, s->c, data_layer);
        appendLayerSim(s, l);
        TEMPLATE(deleteVectorNoDestroy,pMatrix)(data_layer);
        net_size += l->N;
    }
    s->net_size = net_size;
    TEMPLATE(deleteVector,pMatrix)(data);
}

void saveLayersToFile(Sim *s, const char *model_fname) {
    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();
    for(size_t li=0; li<s->layers->size; li++) {
        pMatrixVector *data_layer = serializeSRMLayer(s->layers->array[li]);
        assert(data_layer->size == MATRIX_PER_LAYER);
        for(size_t di=0; di < data_layer->size; di++) {
            TEMPLATE(insertVector,pMatrix)(data, data_layer->array[di]);
        }
        TEMPLATE(deleteVectorNoDestroy,pMatrix)(data_layer);
    }
    assert(data->size > 0);
    assert(model_fname);
    saveMatrixList(model_fname, data);
    TEMPLATE(deleteVector,pMatrix)(data);
}

