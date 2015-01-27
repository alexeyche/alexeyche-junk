#ifndef MOD_H
#define MOD_H

#include "RConstants.h"
#include "RSim.h"
#include "RProto.h"
#include "RGammatoneFB.h"

#define STRICT_R_HEADERS
#include <Rcpp.h>

class RSim;
class RConstants;
class RProto;
class RGammatoneFB;

RCPP_EXPOSED_CLASS(RSim)
RCPP_EXPOSED_CLASS(RConstants)
RCPP_EXPOSED_CLASS(RProto)
RCPP_EXPOSED_CLASS(RGammatoneFB)

#endif
