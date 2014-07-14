#ifndef SERIALIZE_H
#define SERIALIZE_H

typedef struct ModelHead {
    indVector *M;
    indVector *N;
    learning_rule_t rule_types;
    neuron_layer_t layer_types;
    size_t num_of_layers;
} ModelHead;


// serialize
void loadLayersFromFile(Sim *s, const char *model_fname);
void saveLayersToFile(Sim *s, const char *model_file);
void writeModelHead(Sim *s, FileStream *fs);
ModelHead* readModelHead(Sim *s, FileStream *fs);

#endif
