#ifndef SERIALIZE_H
#define SERIALIZE_H



// serialize
void loadLayersFromFile(Sim *s, const char *model_fname, Constants *c, unsigned char statLevel);
void saveLayersToFile(Sim *s, const char *model_file);


#endif
