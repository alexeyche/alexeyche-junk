#ifndef RUN_H
#define RUN_H

#include <time.h>

#include <util/matrix.h>
#include <util/spikes_list.h>

#include "adex.h"

SpikePatternsList* runNeurons(pMatrixVector *ts_data, doubleVector *ts_labels, Constants *c, bool saveStat);

#endif
