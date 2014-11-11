#ifndef PREPARE_TS_H
#define PREPARE_TS_H


#include <float.h>
#include <math.h>

#include <core/constants.h>
#include <core/util/matrix.h>

pMatrixVector* processTimeSeriesSet(pMatrixVector *v, const Constants *c);
Matrix* processThroughReceptiveFields(Matrix *ts, const double *centers, const Constants *c);


#endif
