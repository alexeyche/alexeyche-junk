#ifndef CONNECTION_H
#define CONNECTION_H


#include "basic_elements.h"

class Connection : public SimBasic {
public:		
	Connection(SimElemCommon* pre, SimElemCommon* post);
	void computeMe(double dt);	
	
	mat connMat;
	vec &in;
	vec &out;
};


#endif