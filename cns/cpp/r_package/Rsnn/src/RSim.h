#ifndef RSIM_H
#define RSIM_H

#include <Rcpp.h>


#include "RConstants.h"

#include <snnlib/sim/sim.h>


class RSim : public Sim {
public:
    RSim(RConstants *rc) : Sim(*rc)  {}
    ~RSim() { }

};



#endif
