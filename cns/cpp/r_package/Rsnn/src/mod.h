#ifndef MOD_H
#define MOD_H

#include "RConstants.h"
#include "RSim.h"
#include "RProto.h"

#define STRICT_R_HEADERS
#include <Rcpp.h>

class RSim;
class RConstants;
class RProto;

RCPP_EXPOSED_CLASS(RSim)
RCPP_EXPOSED_CLASS(RConstants)
RCPP_EXPOSED_CLASS(RProto)

#endif
