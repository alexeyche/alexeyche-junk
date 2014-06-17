#ifndef CONFIGURE_H
#define CONFIGURE_H


void configureLayersSim(Sim *s, Constants *c, unsigned char statLevel);

void configureNetSpikesSim(Sim *s, const char *input_spikes_filename, Constants *c);
void configureSimAttr(Sim *s);
void configureSynapses(Sim *s, Constants *c);
void configureRewardModulation(Sim *s);

#endif
