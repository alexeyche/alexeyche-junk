

#include <sim/sim.h>

void writeModelHead(Sim *s, FileStream *fs) {
    const Constants *c = s->ctx->c;
    ModelHead *mh = (ModelHead*) malloc( sizeof(ModelHead) );
    mh->num_of_layers = c->layers_size->size;
    
}

ModelHead* readModelHead(Sim *s, FileStream *fs) {
}



void loadLayersFromFile(Sim *s, const char *model_fname) {
    const Constants *c = s->ctx->c;
    
    FileStream *fs = createInputFileStream(model_fname);
    assert(s->layers->size > 0);
    for(size_t li=0; li< s->layers->size; li++) {
        Layer *l = s->layers->array[li];
        l->deserializeLayer(l, fs, c);
    }
    deleteFileStream(fs);
}

void saveLayersToFile(Sim *s, const char *model_fname) {
    const Constants *c = s->ctx->c;
    
    FileStream *fs = createOutputFileStream(model_fname);
    for(size_t li=0; li<s->layers->size; li++) {
        Layer *l = s->layers->array[li];
        l->serializeLayer(l, fs, c);
    }
    deleteFileStream(fs);
}

