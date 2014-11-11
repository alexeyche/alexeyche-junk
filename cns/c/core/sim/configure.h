#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <core/constants.h>
#include <core/sim/runtime.h>
#include <core/layers/wta.h>
#include <core/layers/wtaadapt.h>
#include <core/layers/adapt.h>

struct Sim;
struct SimImpl;

void configureLayersSim(struct Sim *s, Constants *c);
void configureNetSpikesSim(struct Sim *s, const Constants *c);
void configureSimImpl(struct Sim *s);
void configureSynapses(struct Sim *s, Constants *c);
void configureRewardModulation(struct Sim *s);

#endif
