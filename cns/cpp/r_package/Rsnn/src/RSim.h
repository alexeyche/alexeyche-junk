#ifndef RSIM_H
#define RSIM_H


#include <snnlib/sim/sim.h>

#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "RConstants.h"



class RSim : public Sim {
public:
    RSim(RConstants *rc) : Sim(*rc)  {}
    ~RSim() { }

};



#endif
