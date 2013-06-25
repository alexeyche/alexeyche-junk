#ifndef NEURON_MODELS
#define NEURON_MODELS

#include <iostream>
#include <vector>
#include <sim/core.h>

using namespace boost::numeric::odeint;

typedef std::vector< double > state_type;

#define INTEGRATION_STEP 0.01

class neuron_izh {
public:
    neuron_izh( double a = 0.02, double b = 0.2, double c = -65, double d = 6, double V_rest = -70, double treshold = 30, double dt = INTEGRATION_STEP ) : 
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
    void calc() {    	
    	int dim_size = 50;
        double *table = (double*)malloc(sizeof(double)*dim_size*dim_size*dim_size);
        vec V_space = linspace<vec>(-30,30,dim_size);
        vec t_space = linspace<vec>(0,100,dim_size);
        vec u_space = linspace<vec>(-20,10,dim_size);
        

        //double u = -70*b;
        for(size_t vi=0; vi<V_space.n_elem; vi++) {
    		for(size_t ui=0; ui<u_space.n_elem; ui++) {
                for(size_t ti=0; ti<t_space.n_elem; ti++) {
                    state_type x(3);
                    x[0] = V_space(vi);
                    x[1] = u_space(ui);
                    x[2] = 10;
                    do_step(x, t_space(ti));
        		    table[vi*dim_size*dim_size+ui*dim_size+ti] = x[0];
                }    
    	    }
        }

        for(size_t i=40; i<50; i++) {
            std::cout << i << ": " << "|";
            for(size_t j=0; j<20; j++) {
                std::cout << table[dim_size*dim_size*41+i*dim_size+j] << "|";
            }
            std::cout << "\n";
        }
        
        free(table);
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