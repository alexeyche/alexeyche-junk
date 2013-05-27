#ifndef CONNECTION_H
#define CONNECTION_H


#include "basic_elements.h"

class Connection {
public:		
	Connection(SimElemCommon* pre, SimElemCommon* post);
	void computeMe(double dt);

	SimElemCommon *pre;
	SimElemCommon *post;
	mat connMat;
};


#endif