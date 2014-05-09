#ifndef MESSAGES_H
#define MESSAGES_H

#include <inttypes.h> // defines PRIx macros

#include <util/spikes_list.h>
#include <util/util_vector.h>
#include <conn/net_util.h>


struct {
    SpikeList *spikes;
    doubleVector *reset_timeline;
    indVector *pattern_classes;
} MInputSpikes;



void serialize_double(unsigned char* buffer, double x);
void serialize_size_t(unsigned char* buffer, size_t x);


#endif
