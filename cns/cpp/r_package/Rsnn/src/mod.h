#ifndef MOD_H
#define MOD_H

#include "RConstants.h"
#include "RSim.h"
#include "RProtoRead.h"

#define STRICT_R_HEADERS
#include <Rcpp.h>

class RSim;
class RConstants;
class RProtoRead;

RCPP_EXPOSED_CLASS(RSim)
RCPP_EXPOSED_CLASS(RConstants)
RCPP_EXPOSED_CLASS(RProtoRead)

#endif
