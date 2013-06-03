
#include "poisson.h"

Poisson::Poisson(double mHerz_v) : SimElem(0,1), mHerz(mHerz_v), Iout(0), acc(0)
{ 
}

void Poisson::computeMe(double dt) { 
   if(acc >= mHerz) {
        if(acc >= (mHerz + LONG_MS)) {
            Iout=0;
            acc=0;
            return;
        } 
        Iout=I_OUT;        
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
