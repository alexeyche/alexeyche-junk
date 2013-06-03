
#include "poisson.h"

Poisson::Poisson(double mHerz_v, double long_ms, double Iout_value) : SimElem(0,1), mHerz(mHerz_v), Iout(0), Iout_value(Iout_value), long_ms(long_ms), acc(0)
{ 
}

void Poisson::computeMe(double dt) { 
   if(acc >= mHerz) {
        if(acc >= (mHerz + long_ms)) {
            Iout=0;
            acc=0;
            return;
        } 
        Iout=Iout_value;        
    }                
    acc+=dt;
}

void Poisson::setInput(vec in) {

}

vec Poisson::getOutput() {	
    vec out(1);
	out.fill(Iout);    
	return out;
}
