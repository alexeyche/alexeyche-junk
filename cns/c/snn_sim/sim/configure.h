#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <constants.h>

struct Sim;
struct SimImpl;

void configureLayersSim(struct Sim *s, Constants *c, unsigned char statLevel);
void configureNetSpikesSim(struct Sim *s, const char *input_spikes_filename, Constants *c);
void configureSimAttr(struct SimImpl *s);
void configureSynapses(struct Sim *s, Constants *c);
void configureRewardModulation(struct Sim *s);

#endif
