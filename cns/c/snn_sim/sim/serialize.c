

#include <sim/sim.h>

#define MATRIX_PER_LAYER 6
void loadLayersFromFile(Sim *s, const char *model_fname, Constants *c, unsigned char statLevel) {
    pMatrixVector* data = readMatrixList(model_fname);
    assert(data != NULL);
    assert(s->layers->size > 0);
    for(size_t li=0; li< s->layers->size; li++) {
        pMatrixVector* data_layer = TEMPLATE(createVector,pMatrix)();
        for(size_t di=0; di < MATRIX_PER_LAYER; di++) {
            TEMPLATE(insertVector,pMatrix)(data_layer, data->array[di + MATRIX_PER_LAYER*li]);
        }
        Layer *l = s->layers->array[li];
        l->deserializeLayer(l, s->ctx->c, data_layer);
        TEMPLATE(deleteVectorNoDestroy,pMatrix)(data_layer);
    }
    TEMPLATE(deleteVector,pMatrix)(data);
}

void saveLayersToFile(Sim *s, const char *model_fname) {
    pMatrixVector *data = TEMPLATE(createVector,pMatrix)();
    for(size_t li=0; li<s->layers->size; li++) {
        Layer *l = s->layers->array[li];
        pMatrixVector *data_layer = l->serializeLayer(l);
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

