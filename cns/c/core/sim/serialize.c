

#include <core/sim/sim.h>




void loadLayersFromFile(Sim *s, const char *model_fname) {
    FileStream *fs = createInputFileStream(model_fname);
    assert(s->layers->size > 0);
    for(size_t li=0; li< s->layers->size; li++) {
        LayerPoisson *l = s->layers->array[li];
        l->deserializeLayer(l, fs, s);
    }
    deleteFileStream(fs);
}

void saveLayersToFile(Sim *s, const char *model_fname) {
    checkIdxFnameOfFile(model_fname);
    
    FileStream *fs = createOutputFileStream(model_fname);
    for(size_t li=0; li<s->layers->size; li++) {
        LayerPoisson *l = s->layers->array[li];
        l->serializeLayer(l, fs, s);
    }
    deleteFileStream(fs);
}

