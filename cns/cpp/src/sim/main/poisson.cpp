
#include "poisson.h"

Poisson::Poisson(double mHerz_v, double long_ms, double Iout_value) : SimElem(0,1), mHerz(mHerz_v), Iout(1), Iout_value(Iout_value), long_ms(long_ms), acc(0)
{ 
}

void Poisson::computeMe(double dt) { 
   if(acc >= mHerz) {
        if(acc >= (mHerz + long_ms)) {
            Iout.fill(0);
            acc=0;
            return;
        } 
        Iout.fill(Iout_value);
    }                
    acc+=dt;
}

vec& Poisson::getInput() {
    
}

vec& Poisson::getOutput() {	
	return Iout;
}
