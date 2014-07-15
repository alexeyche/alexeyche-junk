

#include <sim/sim.h>

void writeModelHead(Sim *s, FileStream *fs) {
    const Constants *c = s->ctx->c;
    ModelHead *mh = (ModelHead*) malloc( sizeof(ModelHead) );
    mh->num_of_layers = c->lc->size;
    mh->neuron_types = (neuron_layer_t*) malloc( sizeof(neuron_layer_t) * mh->num_of_layers); 
    mh->learn_types = (learning_rule_t*) malloc( sizeof(learning_rule_t) * mh->num_of_layers); 
    for(size_t li=0; li<c->lc->size; li++) {
        mh->neuron_types[li] = c->lc[li]->neuron_type;
        mh->learn_types[li] = c->lc[li]->learning_rule;
    }
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

