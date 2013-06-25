#ifndef NEURON_MODELS
#define NEURON_MODELS

#include <iostream>
#include <vector>
#include <sim/core.h>

using namespace boost::numeric::odeint;

typedef std::vector< double > state_type;

#define INTEGRATION_STEP 0.01




class Model {
public:
    virtual void do_step(state_type &x, double t) = 0;    
};


class NeuronIzh : public Model {
public:
    NeuronIzh( double a = 0.02, double b = 0.2, double c = -65, double d = 6, double V_rest = -70, double treshold = 30, double dt = INTEGRATION_STEP ) : 
    				a(a), b(b), c(c), d(d), V_rest(V_rest), treshold(treshold), fired(false), dt(dt) { }

    void operator () ( const state_type &x , state_type &dxdt , const double /*t*/ )
    {
    	dxdt[0] = 0.04*x[0]*x[0] + 5*x[0] - V_rest*2 - x[1] + x[2];    
    	dxdt[1] = a * (b*x[0] - x[1]);  
        dxdt[2] = 0;  	
    }

    void do_step(state_type &x, double t) {    //, state_type &out) {
		if(fired) {
			x[0] = c;
			x[1] += d;
			fired=false;
		}
    	stepper.do_step(*this, x, t, dt);
    	if(x[0]>=treshold) {
    		x[0] = treshold;    		
    		fired=true;
    	}    	
    }


private:
	double a;
    double b;
    double c;
    double d;
    double V_rest;
    double treshold;
	
	bool fired;    
	runge_kutta_dopri5< state_type > stepper;
	double dt;
    
};





#endif