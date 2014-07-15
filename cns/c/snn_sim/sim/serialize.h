#ifndef SERIALIZE_H
#define SERIALIZE_H

typedef struct ModelHead {
    neuron_layer_t *neuron_types;
    learning_rule_t *learn_types;
    size_t num_of_layers;
} ModelHead;


// serialize
void loadLayersFromFile(Sim *s, const char *model_fname);
void saveLayersToFile(Sim *s, const char *model_file);
void writeModelHead(Sim *s, FileStream *fs);
ModelHead* readModelHead(Sim *s, FileStream *fs);

#endif
