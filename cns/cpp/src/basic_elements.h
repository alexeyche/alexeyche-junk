#ifndef BASIC_ELEMENTS_H
#define BASIC_ELEMENTS_H

#include "core.h"

struct SimOptions {
    SimOptions(double tau_ms, double time_ms) : tau_ms(tau_ms), time_ms(time_ms), iterations(time_ms/tau_ms) {}
    double tau_ms;
    double time_ms;
    int iterations;
};

struct SimElemSize {
	SimElemSize(int n_in_v, int n_out_v) : n_in(n_in_v), n_out(n_out_v) {}
	int n_in;
	int n_out;
};

class SimBasic {
public:		
	virtual void computeMe(double dt) = 0;	
    virtual void prepareMe(SimOptions so) {}
};

template <typename T_IN, typename T_OUT>
class SimElem : public SimBasic {
    public:            
        SimElem(int n_in = 0, int n_out = 0) : size(n_in, n_out) {}        
        virtual void setInput(T_IN in) = 0;
        virtual T_OUT getOutput() = 0;

        SimElemSize size;
};

typedef SimElem<vec,vec> SimElemCommon;

//---------------------------------------------------------


#endif