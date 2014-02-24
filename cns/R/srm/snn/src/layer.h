#ifndef LAYER_H
#define LAYER_H


#include "netsim.h"

class Layer {
public:
    Layer(const int &N_) : N(N_) {}
    virtual void simdt(const double &t, const double &dt, const List &c, NetSim &n) = 0;
    virtual void prepare(const List &c) = 0;
    bool saveStat;
    int N;
};


#endif
