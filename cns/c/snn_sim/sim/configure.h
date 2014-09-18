#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <constants.h>
#include <sim/runtime.h>
#include <layers/wta.h>
#include <layers/wtaadapt.h>
#include <layers/adapt.h>

struct Sim;
struct SimImpl;

void configureLayersSim(struct Sim *s, Constants *c);
void configureNetSpikesSim(struct Sim *s, Constants *c);
void configureSimImpl(struct Sim *s);
void configureSynapses(struct Sim *s, Constants *c);
void configureRewardModulation(struct Sim *s);

#endif
